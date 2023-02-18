#include "instruction.h"
#include "fntable.h"
#include "log.h"
#include "utils.h"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <utility>

VM::VM(std::string main_fn_id) {
	m_static_area			= nullptr;
	m_static_area_size		= 0;
	m_static_area_used_size = 0;

	const uint64_t STACK_SIZE = 1024 * 1024 * 10;
	m_stack_data_begin		  = new uint8_t[STACK_SIZE];
	m_stack_data_end		  = m_stack_data_begin + STACK_SIZE;
	m_stack_top				  = m_stack_data_begin;

	// 初始化寄存器
	for (size_t i = 0; i < 1 + 8; i++) {
		m_registers.push_back(Register{
			.value	  = 0,
			.is_empty = true,
		});
	}

	// 初始化sr
	m_registers.at(REGISTER_ID_STACK_FRAME).is_empty = false;
	SetRegister(REGISTER_ID_STACK_FRAME, &m_stack_top, sizeof(m_stack_top));

	FnInstructionMaker entry_maker("entry");
	entry_maker.SetFnComment(InstructionComment(0, sprintf_to_stdstr("fn name[entry]")));
	// call main函数的指令
	{
		// var exit_code i32 = 0;
		Var		   var_exitcode = entry_maker.VarBegin("exitcode", 4);
		RegisterId reg_exitcode = AllocGeneralRegister();
		entry_maker.AddInstruction(new Instruction_add_const<uint64_t, true, true>(entry_maker, reg_exitcode, REGISTER_ID_STACK_FRAME, var_exitcode.mem_addr.relative_addr));
		entry_maker.AddInstruction(new Instruction_write_const_value<uint32_t>(entry_maker, reg_exitcode, 0));

		ReleaseGeneralRegister(reg_exitcode);

		// exit_code = main(id);
		entry_maker.AddInstruction(new Instruction_call(entry_maker, main_fn_id, entry_maker.GetVar("exitcode").mem_addr.relative_addr));

		reg_exitcode = AllocGeneralRegister();
		entry_maker.AddInstruction(new Instruction_add_const<uint64_t, true, true>(entry_maker, reg_exitcode, REGISTER_ID_STACK_FRAME, var_exitcode.mem_addr.relative_addr));
		entry_maker.AddInstruction(new Instruction_exit(entry_maker, reg_exitcode));
		ReleaseGeneralRegister(reg_exitcode);
	}

	entry_maker.Finish();
	AddFn(entry_maker);
}
MemAddr VM::AddStaticData(const void* data, uint64_t size) {
	if (m_static_area_used_size + size > m_static_area_size) {
		m_static_area_size		 = m_static_area_used_size + size;
		uint8_t* new_static_data = new uint8_t[m_static_area_size];
		if (m_static_area != nullptr) {
			memcpy(new_static_data, m_static_area, m_static_area_used_size);
		}
		m_static_area = new_static_data;
	}
	memcpy(m_static_area + m_static_area_used_size, data, size);
	uint64_t offset = m_static_area_used_size;
	m_static_area_used_size += size;

	return MemAddr(MemAddr::RELATIVE_TO_STATIC_AREA, offset);
}
void VM::AddFn(const FnInstructionMaker& maker) {
	if (m_fn_id_2_instruction_addr_offset.find(maker.GetFnId()) != m_fn_id_2_instruction_addr_offset.end()) {
		panicf("duplicate fn_id[%s]", maker.GetFnId().c_str());
	}
	maker.InsertComments(m_instructions.size(), m_comments);
	uint64_t fn_instruction_addr_offset = m_instructions.size() * sizeof(void*);
	for (auto i : maker.GetInstructions()) {
		m_instructions.push_back(i);
	}
	m_fn_id_2_instruction_addr_offset[maker.GetFnId()] = fn_instruction_addr_offset;
}
void VM::RefreshFnAddr() {
	for (auto i : m_instructions) {
		Instruction_call* call = dynamic_cast<Instruction_call*>(i);
		if (call != nullptr) {
			std::string fnid  = call->GetFnId();
			auto		found = m_fn_id_2_instruction_addr_offset.find(fnid);
			if (found == m_fn_id_2_instruction_addr_offset.end()) {
				panicf("unknown fnid[%s]", fnid.c_str());
			}
			call->SetFnInstructionAddrOffset(found->second);
		}
	}
}
void VM::Finish() {
	RefreshFnAddr();
	prepare_all();
}
void VM::MakeAbsoluteMemAddr_load(MemAddr& addr) {
	switch (addr.type) {
	case MemAddr::RELATIVE_TO_STATIC_AREA:
		addr.absolute_addr = (uint64_t)m_static_area + addr.relative_addr;
		break;
	case MemAddr::RELATIVE_TO_INSTRUCTION_AREA:
		addr.absolute_addr = (uint64_t)&m_instructions[0] + addr.relative_addr;
		break;
	case MemAddr::RELATIVE_TO_STACK_AREA: // 运行时动态改变, 无法在执行前固化
		break;
	default:
		panicf("bug");
	}
}
void VM::MakeAbsoluteMemAddr_run(MemAddr& addr) {
	switch (addr.type) {
	case MemAddr::RELATIVE_TO_STATIC_AREA: // 执行之前地址固定, 因此在load阶段处理
		break;
	case MemAddr::RELATIVE_TO_INSTRUCTION_AREA: // 执行之前地址固定, 因此在load阶段处理
		break;
	case MemAddr::RELATIVE_TO_STACK_AREA:
		addr.absolute_addr = GetRegister<uint64_t>(REGISTER_ID_STACK_FRAME) + addr.relative_addr;
		break;
	case MemAddr::STATIC_FN_ID: // 静态函数地址在编译阶段固定
		break;
	default:
		panicf("bug");
	}
}
uint8_t* VM::AllocStack(int bytes) {
	if (m_stack_top + bytes >= m_stack_data_end) {
		panicf("stack overflow");
	}

	uint8_t* ptr = m_stack_top;
	m_stack_top += bytes;

	return ptr;
}
void VM::Push(uint64_t value) {
	*(uint64_t*)m_stack_top = value;
	m_stack_top += 8;
}
void VM::Pop(uint64_t bytes) {
	assert(bytes < (uint64_t)m_stack_top);
	if (m_stack_top - bytes <= m_stack_data_begin) {
		panicf("stack overflow");
	}
	m_stack_top -= bytes;
}
void VM::prepare_all() {
	for (auto instruction : m_instructions) {
		instruction->Prepare(this);
	}
}
void VM::PrintInstructions() const {
	uint64_t instruction_offset = 0;
	for (size_t i = 0; i < m_instructions.size(); i++) {
		auto found = m_comments.find(i);
		if (found != m_comments.end()) {
			printf("%s\n", found->second.GetContent().c_str());
		}
		printf("%4lu %s\n", instruction_offset, m_instructions.at(i)->GetDesc().c_str());
		instruction_offset += 8;
	}
}
void VM::Start() {
	m_reg_instruction = (uint64_t)&m_instructions[0];
	while (m_reg_instruction != 0) {
		Instruction* cur_instruction = *(Instruction**)m_reg_instruction;
		log_debug("execute instruction: %s\n", cur_instruction->GetDesc().c_str());
		cur_instruction->Execute();
	}
}
void VM::Inc_ir() {
	m_reg_instruction += sizeof(long);
}
const Var FnInstructionMaker::GetVar(std::string var_name) const {
	for (auto& v : m_vars) {
		if (v.var_name == var_name) {
			return v;
		}
	}
	panicf("unknown var_name[%s] in fn[%s]", var_name.c_str(), m_fn_id.c_str());
}
bool FnInstructionMaker::HasVar(std::string var_name) const {
	for (auto& v : m_vars) {
		if (v.var_name == var_name) {
			return true;
		}
	}
	return false;
}
void FnInstructionMaker::SetArg(std::string var_name, uint64_t mem_size, MemAddr mem_addr) {
	assert(!HasVar(var_name));
	Var new_var{
		.var_name = var_name,
		.mem_size = mem_size,
		.mem_addr = mem_addr,
	};
	m_vars.push_back(new_var);
}
void FnInstructionMaker::AddInstruction(Instruction* instruction) {
	instruction->SetFnId(m_fn_id);
	m_instructions.push_back(instruction);
}
Var FnInstructionMaker::VarBegin(std::string var_name, uint64_t size) {
	// TODO memory alignment
	assert(!HasVar(var_name));
	Var new_var{
		.var_name = var_name,
		.mem_size = size,
		.mem_addr = MemAddr(MemAddr::RELATIVE_TO_STACK_AREA, m_cur_offset),
	};
	m_vars.push_back(new_var);
	m_cur_offset += size;
	m_max_offset = std::max(m_max_offset, m_cur_offset);
	return new_var;
}
Var FnInstructionMaker::TmpVarBegin(std::string prefix, uint64_t size) {
	// TODO memory alignment
	std::string var_name = sprintf_to_stdstr("tmp_%s_%d", prefix.c_str(), m_tmp_var_name_seed);
	m_tmp_var_name_seed++;
	Var var				  = VarBegin(var_name, size);
	var.mem_addr.tmp_name = var_name;
	return var;
}
void FnInstructionMaker::VarEnd(std::string var_name) {
	for (auto iter = m_vars.begin(); iter != m_vars.end(); iter++) {
		if (iter->var_name == var_name) {
			assert(iter->mem_size < m_cur_offset);
			m_cur_offset -= iter->mem_size;
			m_vars.erase(iter);
			return;
		}
	}
	panicf("var %s not exists", var_name.c_str());
}
const Var FnInstructionMaker::GetVarByStackOffset(int64_t stack_offset) const {
	// 栈偏移恰好相等
	for (auto iter = m_vars.begin(); iter != m_vars.end(); iter++) {
		assert(iter->mem_addr.type == MemAddr::RELATIVE_TO_STACK_AREA);
		if (iter->mem_addr.relative_addr == stack_offset) {
			return *iter;
		}
	}
	// 栈偏移在变量的栈内存范围内. 比如数组的某个元素
	for (auto iter = m_vars.begin(); iter != m_vars.end(); iter++) {
		assert(iter->mem_addr.type == MemAddr::RELATIVE_TO_STACK_AREA);
		if (iter->mem_addr.relative_addr < stack_offset && iter->mem_addr.relative_addr + (int64_t)iter->mem_size > stack_offset) {
			return *iter;
		}
	}
	panicf("stack-offset %ld not exists", stack_offset);
}
void FnInstructionMaker::Finish() {
	log_debug("fn[%s] stack max offset[%lu]", m_fn_id.c_str(), m_max_offset);
	assert(m_max_offset >= START_OFFSET);
	if (m_max_offset > START_OFFSET) {
		Instruction* instruction = new Instruction_stack_alloc(m_max_offset - START_OFFSET);
		instruction->SetFnId(m_fn_id);
		m_instructions.insert(m_instructions.begin(), instruction);

		// 插入了一条指令, 修正指令和comment的对应关系
		std::map<size_t, InstructionComment> tmp_comment;
		for (auto c : m_comments) {
			tmp_comment[c.first + 1] = c.second;
		}
		m_comments = tmp_comment;
	}
	if (!m_fn_comment.GetContent().empty()) {
		auto found = m_comments.find(0);
		if (found == m_comments.end()) {
			m_comments[0] = m_fn_comment;
		} else {
			m_fn_comment.Merge(found->second);
			found->second = m_fn_comment;
		}
	}
}
Instruction_call::Instruction_call(FnInstructionMaker& maker, std::string fn_id, int64_t return_var_offset) {
	m_fn_id						 = fn_id;
	m_is_fn_instruction_addr_set = false;
	m_return_var_offset			 = return_var_offset;
	m_desc						 = sprintf_to_stdstr("call fn[%s] return_var_offset[%d]", m_fn_id.c_str(), m_return_var_offset);
}
void Instruction_call::Execute() {
	assert(m_is_fn_instruction_addr_set);
	uint64_t caller_sr = m_vm->GetRegister<uint64_t>(REGISTER_ID_STACK_FRAME);
	uint64_t caller_ir = m_vm->GetReg_ir();
	uint64_t caller_rr = m_vm->GetReg_rr();

	uint64_t callee_sr = (uint64_t)m_vm->GetStackTop();

	uint64_t rr_absolute_addr = caller_sr + m_return_var_offset;

	m_vm->Push(caller_sr);
	m_vm->Push(caller_ir + 8); // 指向call的下一条指令
	m_vm->Push(caller_rr);

	m_vm->SetRegister(REGISTER_ID_STACK_FRAME, &callee_sr, sizeof(caller_sr));
	m_vm->SetReg_ir(m_fn_instruction_addr);
	m_vm->SetReg_rr(rr_absolute_addr);
}
void Instruction_call::Prepare(VM* vm) {
	assert(m_is_fn_instruction_addr_set);
	m_vm				  = vm;
	m_fn_instruction_addr = vm->GetInstructionAreaStartAddr() + m_fn_instruction_addr_offset;
}
std::string MemAddr::ToString(FnInstructionMaker& maker) const {
	char		buf[128];
	const char* prefix = "";
	if (is_pointer) {
		prefix = "*";
	}
	switch (type) {
	case RELATIVE_TO_STACK_AREA:
	{
		const Var var = maker.GetVarByStackOffset(relative_addr);
		if (relative_addr >= 0) {
			snprintf(buf, sizeof(buf), "%s%s:sr+%ld", prefix, var.var_name.c_str(), relative_addr);
		} else {
			snprintf(buf, sizeof(buf), "%s%s:sr%ld", prefix, var.var_name.c_str(), relative_addr);
		}
		break;
	}
	case RELATIVE_TO_STATIC_AREA:
		snprintf(buf, sizeof(buf), "%sstatic-area+%ld", prefix, relative_addr);
		break;
	case RELATIVE_TO_INSTRUCTION_AREA:
		snprintf(buf, sizeof(buf), "%sinstruction-area %ld", prefix, relative_addr);
		break;
	case STATIC_FN_ID:
		snprintf(buf, sizeof(buf), "static-fn %s", fn_id.c_str());
		break;
	default:
		panicf("bug");
	}
	return std::string(buf);
}
void VM::SetRegister(RegisterId rid, const void* ptr, size_t bytes) {
	//assert(m_registers.at(rid).is_empty == false);
	assert(bytes <= sizeof(Register::value));
	memcpy((void*)&(m_registers.at(rid).value), ptr, bytes);
}
RegisterId VM::AllocGeneralRegister() {
	for (size_t i = 0; i < m_registers.size(); i++) {
		if (m_registers.at(i).is_empty) {
			m_registers.at(i).is_empty = false;
			return (RegisterId)i;
		}
	}
	panicf("no empty register to allocate");
}
void VM::ReleaseGeneralRegister(RegisterId rid) {
	assert(m_registers.at(rid).is_empty == false);
	m_registers.at(rid).is_empty = true;
}
const void* VM::GetRegisterAddr(RegisterId rid) {
	//assert(m_registers.at(rid).is_empty == false);
	return (const void*)&m_registers.at(rid).value;
}
bool CompileResult::IsFnId() const {
	assert(m_unset == false);
	return m_fn_id.empty() == false;
}
std::string CompileResult::GetFnId() const {
	return m_fn_id;
}
RegisterId CompileResult::GetRegisterId() const {
	assert(m_unset == false && m_fn_id.empty());
	return m_rid;
}
bool CompileResult::IsValue() const {
	assert(m_unset == false && m_fn_id.empty());
	return m_is_value;
}
std::string CompileResult::GetStackVarName() const {
	assert(m_unset == false && m_fn_id.empty());
	return m_stack_var_name;
}
bool FnInstructionMaker::HasInstruction(const Instruction* instruction) const {
	for (size_t i = 0; i < m_instructions.size(); i++) {
		if (m_instructions[i] == instruction) {
			return true;
		}
	}
	return false;
}
void FnInstructionMaker::AddComment(InstructionComment comment) {
	auto found = m_comments.find(m_instructions.size());
	if (found == m_comments.end()) {
		m_comments[m_instructions.size()] = comment;
	} else {
		found->second.Merge(comment);
	}
}
void FnInstructionMaker::SetFnComment(InstructionComment comment) {
	m_fn_comment = comment;
}
void FnInstructionMaker::InsertComments(size_t fn_start_idx, std::map<size_t, InstructionComment>& comments) const {
	for (auto iter = m_comments.begin(); iter != m_comments.end(); iter++) {
		int idx = fn_start_idx + iter->first;
		assert(comments.find(idx) == comments.end());
		comments[idx] = iter->second;
	}
}
void InstructionComment::Merge(const InstructionComment& another) {
	m_content += "\n" + another.m_content;
}
