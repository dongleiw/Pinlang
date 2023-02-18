#include "instruction.h"
#include "fntable.h"
#include "log.h"
#include "utils.h"
#include <algorithm>
#include <cassert>
#include <cstring>

VM::VM(std::string main_fn_id) {
	m_static_area			= nullptr;
	m_static_area_size		= 0;
	m_static_area_used_size = 0;

	const uint64_t STACK_SIZE = 1024 * 1024 * 10;
	m_stack_data_begin		  = new uint8_t[STACK_SIZE];
	m_stack_data_end		  = m_stack_data_begin + STACK_SIZE;
	m_stack_top				  = m_stack_data_begin;

	m_reg_stack_frame = (uint64_t)m_stack_top;

	FnInstructionMaker entry_maker("entry");
	// call main函数的指令
	{
		// var exit_code i32 = 0;
		entry_maker.VarBegin("exitcode", 4);
		entry_maker.AddInstruction(new Instruction_write_const_value<uint32_t>(entry_maker.GetVar("exitcode"), 0));

		// main arg: push(100, i32)
		entry_maker.VarBegin("arg_a", 4);
		entry_maker.AddInstruction(new Instruction_write_const_value<uint32_t>(entry_maker.GetVar("arg_a"), 100));

		// exit_code = main(id);
		entry_maker.AddInstruction(new Instruction_call(main_fn_id, entry_maker.GetVar("exitcode")));

		entry_maker.AddInstruction(new Instruction_exit(entry_maker.GetVar("exitcode")));
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
	if (m_fn_id_2_instruction_addr.find(maker.GetFnId()) != m_fn_id_2_instruction_addr.end()) {
		panicf("duplicate fn_id[%s]", maker.GetFnId().c_str());
	}
	MemAddr fn_instruction_addr(MemAddr::RELATIVE_TO_INSTRUCTION_AREA, m_instructions.size() * sizeof(void*));
	for (auto i : maker.GetInstructions()) {
		m_instructions.push_back(i);
	}
	m_fn_id_2_instruction_addr[maker.GetFnId()] = fn_instruction_addr;
}
void VM::RefreshFnAddr() {
	for (auto i : m_instructions) {
		Instruction_call* call = dynamic_cast<Instruction_call*>(i);
		if (call != nullptr) {
			std::string fnid  = call->GetFnId();
			auto		found = m_fn_id_2_instruction_addr.find(fnid);
			if (found == m_fn_id_2_instruction_addr.end()) {
				panicf("unknown fnid[%s]", fnid.c_str());
			}
			call->SetInstructionAddr(found->second);
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
	case MemAddr::PTR_RELATIVE_TO_STACK_AREA: // 运行时动态改变, 无法在执行前固化
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
		addr.absolute_addr = m_reg_stack_frame + addr.relative_addr;
		break;
	case MemAddr::PTR_RELATIVE_TO_STACK_AREA:
	{
		uint64_t pointer_addr = (uint64_t)m_reg_stack_frame + addr.relative_addr;
		addr.absolute_addr	  = *(uint64_t*)pointer_addr;
		break;
	}
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
void VM::test1() {
	FnInstructionMaker entry_maker("entry");
	// call main函数的指令
	{
		// var exit_code i32 = 0;
		entry_maker.VarBegin("exitcode", 4);
		entry_maker.AddInstruction(new Instruction_write_const_value<uint32_t>(entry_maker.GetVar("exitcode"), 0));

		// main arg: push(100, i32)
		entry_maker.VarBegin("arg_a", 4);
		entry_maker.AddInstruction(new Instruction_write_const_value<uint32_t>(entry_maker.GetVar("arg_a"), 100));

		// exit_code = main(id);
		entry_maker.AddInstruction(new Instruction_call("main", entry_maker.GetVar("exitcode")));

		entry_maker.AddInstruction(new Instruction_exit(entry_maker.GetVar("exitcode")));
	}

	entry_maker.Finish();
	AddFn(entry_maker);

	// 函数main的指令
	FnInstructionMaker main_maker("main");
	{
		/*
		 fn main(a i32) i32{
				{
					var i i32=1;
				}
				{
					var i i32=1;
				}
				{
					var i i32=1;
				}
				return 0;
		 }
		 */

		MemAddr addr_arg_a(MemAddr::RELATIVE_TO_STACK_AREA, -4);

		main_maker.VarBegin("i1", 8);
		main_maker.AddInstruction(new Instruction_write_const_value<uint32_t>(main_maker.GetVar("i1"), 1));
		main_maker.VarEnd("i1");

		main_maker.VarBegin("i2", 8);
		main_maker.AddInstruction(new Instruction_write_const_value<uint32_t>(main_maker.GetVar("i2"), 1));
		main_maker.VarEnd("i2");

		main_maker.VarBegin("i3", 8);
		main_maker.AddInstruction(new Instruction_write_const_value<uint32_t>(main_maker.GetVar("i3"), 1));
		main_maker.VarEnd("i3");

		// return 0;
		main_maker.AddInstruction(new Instruction_ret_const_value<int32_t>(100));
	}

	main_maker.Finish();
	AddFn(main_maker);

	m_reg_instruction = (uint64_t)&m_instructions[0];

	m_reg_return_mem_address = 0; // 相对于callee的rr

	Finish();
}
void VM::prepare_all() {
	for (auto instruction : m_instructions) {
		instruction->Prepare(this);
	}
}
void VM::PrintInstructions() const {
	std::string cur_fn_id;
	uint64_t	instruction_offset = 0;
	for (auto instruction : m_instructions) {
		if (cur_fn_id != instruction->GetFnId()) {
			cur_fn_id = instruction->GetFnId();
			printf("### fn[%s]\n", cur_fn_id.c_str());
		}
		printf("%4lu %s\n", instruction_offset, instruction->GetDesc().c_str());
		instruction_offset += 8;
	}
}
void VM::Start() {
	m_reg_instruction = (uint64_t)&m_instructions[0];
	while (m_reg_instruction != 0) {
		Instruction* cur_instruction = *(Instruction**)m_reg_instruction;
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
void FnInstructionMaker::AddInstruction(Instruction* instruction) {
	instruction->SetFnId(m_fn_id);
	m_instructions.push_back(instruction);
}
const Var FnInstructionMaker::VarBegin(std::string var_name, uint64_t size) {
	// TODO memory alignment
	assert(!HasVar(var_name));
	Var new_var{
		.var_name	  = var_name,
		.stack_offset = m_cur_offset,
		.mem_size	  = size,
		.mem_addr	  = MemAddr(MemAddr::RELATIVE_TO_STACK_AREA, m_cur_offset),
	};
	m_vars.push_back(new_var);
	m_cur_offset += size;
	m_max_offset = std::max(m_max_offset, m_cur_offset);
	return new_var;
}
const Var FnInstructionMaker::TmpVarBegin(uint64_t size) {
	// TODO memory alignment
	std::string var_name = sprintf_to_stdstr("tmpv_%d", m_tmp_var_name_seed);
	m_tmp_var_name_seed++;
	return VarBegin(var_name, size);
}
void FnInstructionMaker::VarEnd(std::string var_name) {
	assert(HasVar(var_name));
	const Var v = GetVar(var_name);
	assert(v.mem_size < m_cur_offset);
	m_cur_offset -= v.mem_size;
}
void FnInstructionMaker::Finish() {
	log_debug("fn[%s] stack max offset[%lu]", m_fn_id.c_str(), m_max_offset);
	assert(m_max_offset >= START_OFFSET);
	if (m_max_offset > START_OFFSET) {
		Instruction* instruction = new Instruction_stack_alloc(m_max_offset - START_OFFSET);
		instruction->SetFnId(m_fn_id);
		m_instructions.insert(m_instructions.begin(), instruction);
	}
}
Instruction_ret::Instruction_ret(uint64_t return_var_size, MemAddr return_var_addr) {
	assert(return_var_addr.type == MemAddr::RELATIVE_TO_STACK_AREA);
	m_has_return_var  = true;
	m_return_var_size = return_var_size;
	m_return_var_addr = return_var_addr;
	m_desc			 = sprintf_to_stdstr("ret callee-return-var-addr[%s] %lu byte", m_return_var_addr.ToString().c_str(), m_return_var_size);
}
Instruction_ret::Instruction_ret() {
	m_has_return_var = false;
	m_desc			 = sprintf_to_stdstr("ret");
}
void Instruction_ret::Execute() {
	m_vm->MakeAbsoluteMemAddr_run(m_return_var_addr);
	if (m_has_return_var) {
		memcpy((void*)m_vm->GetReg_rr(), (const void*)m_return_var_addr.absolute_addr, m_return_var_size);
	}

	uint64_t caller_sr = *(uint64_t*)m_vm->GetReg_sr();
	uint64_t caller_ir = *((uint64_t*)m_vm->GetReg_sr() + 1);
	uint64_t caller_rr = *((uint64_t*)m_vm->GetReg_sr() + 2);

	m_vm->Pop((uint64_t)m_vm->GetStackTop() - m_vm->GetReg_sr());

	m_vm->SetReg_sr(caller_sr);
	m_vm->SetReg_ir(caller_ir);
	m_vm->SetReg_rr(caller_rr);
}
void Instruction_ret::Prepare(VM* vm) {
	m_vm = vm;
	m_vm->MakeAbsoluteMemAddr_load(m_return_var_addr);
}
