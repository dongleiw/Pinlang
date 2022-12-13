#pragma once

#include "log.h"
#include "utils.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <vector>

class VM;

struct MemAddr {
	enum AddrType {
		RELATIVE_TO_STACK_AREA,		  // 相对当前sr
		RELATIVE_TO_STATIC_AREA,	  // 相对static area的起始地址
		RELATIVE_TO_INSTRUCTION_AREA, // 相对instruction area的起始地址
		PTR_RELATIVE_TO_STACK_AREA,	  // 在`RELATIVE_TO_STACK_AREA`位置读取一个地址
	};
	AddrType type;
	int64_t	 relative_addr;
	uint64_t absolute_addr;

	MemAddr(AddrType a_type, int64_t a_relative_addr) : type(a_type), relative_addr(a_relative_addr), absolute_addr(0) {
	}
	MemAddr() : type(RELATIVE_TO_STACK_AREA), relative_addr(0) {
	}

	std::string ToString() const {
		char buf[128];
		switch (type) {
		case RELATIVE_TO_STACK_AREA:
			snprintf(buf, sizeof(buf), "sr+%ld", relative_addr);
			break;
		case RELATIVE_TO_STATIC_AREA:
			snprintf(buf, sizeof(buf), "static-area %ld", relative_addr);
			break;
		case RELATIVE_TO_INSTRUCTION_AREA:
			snprintf(buf, sizeof(buf), "instruction-area %ld", relative_addr);
			break;
		case PTR_RELATIVE_TO_STACK_AREA:
			snprintf(buf, sizeof(buf), "stack-frame ptr %ld", relative_addr);
			break;
		default:
			panicf("bug");
		}
		return std::string(buf);
	}
};

/*
 * 指令的构造函数中的参数必须都是编译期确定不变的值. 因为指令在执行时, 没有参数传入.
 */
class Instruction {
public:
	Instruction() {}

	virtual void Execute()		 = 0;
	virtual void Prepare(VM* vm) = 0;

	void		SetFnId(std::string fnid) { m_fn_id = fnid; }
	std::string GetFnId() const { return m_fn_id; }

	const std::string& GetDesc() const { return m_desc; };

protected:
	VM*			m_vm;
	std::string m_fn_id;
	std::string m_desc;
};
struct Var {
	std::string var_name;
	uint64_t	stack_offset;
	uint64_t	mem_size;
	MemAddr		mem_addr;
};
struct StaticData {
	std::string name;
	uint64_t	size;
	void*		data;
};

/*
 * 负责生成函数指令
 * 一个函数内的所有变量id必须唯一
 * 栈内存管理
 *		var a i32=1;
 *		{
 *			var b i32=1;
 *		}
 *		{
 *			var c i32=1;
 *		}
 *		c在开始前,b已经结束,因此c可以复用b的内存
 */
class FnInstructionMaker {
public:
	static const uint64_t START_OFFSET = 24;

public:
	FnInstructionMaker(std::string fn_id) {
		m_fn_id		 = fn_id;
		m_cur_offset = START_OFFSET;
		m_max_offset = m_cur_offset;
	}

	// 一个变量开始生命周期
	const Var VarBegin(std::string var_name, uint64_t size);
	const Var TmpVarBegin(uint64_t size);
	// 一个变量结束声明周期. 之后该变量的栈内存就可以释放掉给后续变量使用了
	void VarEnd(std::string var_name);

	const Var GetVar(std::string var_name) const;
	bool	  HasVar(std::string var_name) const;

	void							AddInstruction(Instruction* instruction);
	const std::string&				GetFnId() const { return m_fn_id; }
	const std::vector<Instruction*> GetInstructions() const { return m_instructions; }

	// 函数指令已经生成添加完毕.
	// 生成第一条指令. 一次性分配所有栈内存
	void Finish();

private:
	std::string				  m_fn_id;
	std::vector<Instruction*> m_instructions;
	std::vector<Var>		  m_vars;		// 分配的变量列表. 按照栈增长的方向依次排序
	uint64_t				  m_cur_offset; // 当前已分配的栈内存的偏移
	uint64_t				  m_max_offset; // 记录分配栈内存过程中的最大偏移. 在函数开始位置一次性分配完.
	int						  m_tmp_var_name_seed;
};

/*
 * 内存区域
 *		- stack
 *		- heap
 *		- static area
 *		- instruction area
 * 相对内存地址
 *		在编译时, 由于不知道指令执行时被加载的内存地址, 因此指令中都是相对地址
 *			- instruction area offset
 *			- static area offset
 *			- stack area offset
 *		指令中的内存地址都是相对位置, 在指令之前需要将内存地址从相对地址改为绝对地址
 * 函数地址:
 *		由于有相互依赖问题, call指令设置时, 仅提供fnid, 不设置函数地址.
 *		在所有指令设置完毕后, 再刷新一遍
 */
class VM {
public:
public:
	VM(std::string main_fn_id);

	uint8_t* GetRegStackBase();
	uint8_t* GetReg_mr(int id);
	void	 SetReg_mr(int id, uint8_t* address);
	uint8_t* GetConst(int id);

	void	 Push(uint64_t value);
	void	 Pop(uint64_t bytes);
	uint8_t* AllocStack(int bytes);
	uint8_t* GetStackTop() const { return m_stack_top; }

	uint64_t GetReg_ir() const { return m_reg_instruction; }
	void	 SetReg_ir(uint64_t ir) { m_reg_instruction = ir; }
	void	 Inc_ir();

	uint64_t GetReg_sr() const { return m_reg_stack_frame; }
	void	 SetReg_sr(uint64_t ir) { m_reg_stack_frame = ir; }

	uint64_t GetReg_rr() const { return m_reg_return_mem_address; }
	void	 SetReg_rr(uint64_t rr) { m_reg_return_mem_address = rr; }

	/*
	 * 增加静态数据
	 * 返回该静态数据的相对位置
	 */
	MemAddr AddStaticData(const void* data, uint64_t size);

	void MakeAbsoluteMemAddr_load(MemAddr& addr);
	void MakeAbsoluteMemAddr_run(MemAddr& addr);

	// 指令添加完毕
	//	- 刷新各函数的地址
	//	- 设置入口函数代码
	// main_fn_id 入口函数, 通常都是'main'
	void Finish();

	void Start();

	void PrintInstructions() const;

public:
	void AddFn(const FnInstructionMaker& maker);
	// 指令设置过程中, 函数的调用只是设置了fnid, 需要刷新函数的地址
	void RefreshFnAddr();

public: // test
	void test1();

private:
	void prepare_all();

private:
	std::vector<Instruction*>	   m_instructions;
	std::map<std::string, MemAddr> m_fn_id_2_instruction_addr; // 函数id到函数的指令起始地址的映射关系

	uint8_t* m_static_area;
	uint64_t m_static_area_size;
	uint64_t m_static_area_used_size;

	uint8_t* m_stack_data_begin; //
	uint8_t* m_stack_data_end;
	uint8_t* m_stack_top; // stack栈顶. 未使用的栈内存的起始地址

	/*
	 * sr
	 * 当前栈帧的栈底内存地址
	 */
	uint64_t m_reg_stack_frame;
	/*
	 * ir
	 * 下一条指令的内存地址
	 */
	uint64_t m_reg_instruction;
	/*
	 * rr
	 * 当前函数返回值应该保存的内存地址( 相对sr的stack-offset )
	 */
	uint64_t m_reg_return_mem_address;

	struct MR {
		bool	 empty;
		uint64_t value;
		MR() : empty(true), value(0) {}
	};
	MR m_mr[128]; // mr 通用的内存地址寄存器
};

/*
 * 将指定内存地址的两个数相加(位数支持8/16/32/64), 结果存储到指定内存地址
 */
template <typename T>
class Instruction_add : public Instruction {
public:
	Instruction_add(MemAddr result_addr, MemAddr left_addr, MemAddr right_addr) {
		m_result_addr = result_addr;
		m_left_addr	  = left_addr;
		m_right_addr  = right_addr;
		m_desc=sprintf_to_stdstr("add left[%s] right[%s] result[%s]", left_addr.ToString().c_str(), right_addr.ToString().c_str(), result_addr.ToString().c_str());
	}

	virtual void Execute() override {
		m_vm->MakeAbsoluteMemAddr_run(m_result_addr);
		m_vm->MakeAbsoluteMemAddr_run(m_left_addr);
		m_vm->MakeAbsoluteMemAddr_run(m_right_addr);
		*(T*)m_result_addr.absolute_addr = *(T*)m_left_addr.absolute_addr + *(T*)m_right_addr.absolute_addr;
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
		m_vm->MakeAbsoluteMemAddr_load(m_result_addr);
		m_vm->MakeAbsoluteMemAddr_load(m_left_addr);
		m_vm->MakeAbsoluteMemAddr_load(m_right_addr);
	}

private:
	MemAddr m_result_addr;
	MemAddr m_left_addr;
	MemAddr m_right_addr;
};

/*
 * 将指定内存地址的两个数相乘(位数支持8/16/32/64), 结果存储到指定内存地址
 * 用法
 */
template <typename T>
class Instruction_mul : public Instruction {
public:
	Instruction_mul(MemAddr result_addr, MemAddr left_addr, MemAddr right_addr) {
		m_result_addr = result_addr;
		m_left_addr	  = left_addr;
		m_right_addr  = right_addr;
	}

	virtual void Execute() override {
		m_vm->MakeAbsoluteMemAddr_run(m_result_addr);
		m_vm->MakeAbsoluteMemAddr_run(m_left_addr);
		m_vm->MakeAbsoluteMemAddr_run(m_right_addr);
		*(T*)m_result_addr.absolute_addr = *(T*)m_left_addr.absolute_addr * *(T*)m_right_addr.absolute_addr;
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
		m_vm->MakeAbsoluteMemAddr_load(m_result_addr);
		m_vm->MakeAbsoluteMemAddr_load(m_left_addr);
		m_vm->MakeAbsoluteMemAddr_load(m_right_addr);
	}

private:
	MemAddr m_result_addr;
	MemAddr m_left_addr;
	MemAddr m_right_addr;
};

/*
 * 将指定内存地址的两个数相乘(位数支持8/16/32/64), 结果存储到指定内存地址
 * 用法
 */
template <typename ResultT, typename LeftT, typename RightT>
class Instruction_mul_const : public Instruction {
public:
	Instruction_mul_const(MemAddr result_addr, MemAddr left_addr, RightT right_value) {
		m_result_addr = result_addr;
		m_left_addr	  = left_addr;
		m_right_value = right_value;
	}

	virtual void Execute() override {
		m_vm->MakeAbsoluteMemAddr_run(m_result_addr);
		m_vm->MakeAbsoluteMemAddr_run(m_left_addr);
		*(ResultT*)m_result_addr.absolute_addr = *(LeftT*)m_left_addr.absolute_addr * m_right_value;
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
		m_vm->MakeAbsoluteMemAddr_load(m_result_addr);
		m_vm->MakeAbsoluteMemAddr_load(m_left_addr);
	}

private:
	MemAddr m_result_addr;
	MemAddr m_left_addr;
	RightT	m_right_value;
};

/*
 * 从源地址读取一定长度数据写入到目标地址
 * dst_addr
 */
class Instruction_memcpy_from_value_to_value : public Instruction {
public:
	Instruction_memcpy_from_value_to_value(const Var& dst_var, const Var& src_var) {
		assert(dst_var.mem_size == src_var.mem_size);
		m_dst_addr = dst_var.mem_addr;
		m_src_addr = src_var.mem_addr;
		m_bytes	   = dst_var.mem_size;
		m_desc	   = sprintf_to_stdstr("copy from var[%s] to var[%s]", src_var.var_name.c_str(), dst_var.var_name.c_str());
	}
	Instruction_memcpy_from_value_to_value(const Var& dst_var, const MemAddr& src_addr) {
		m_dst_addr = dst_var.mem_addr;
		m_src_addr = src_addr;
		m_bytes	   = dst_var.mem_size;
		m_desc	   = sprintf_to_stdstr("copy from addr[%s] to var[%s]", src_addr.ToString().c_str(), dst_var.var_name.c_str());
	}

	virtual void Execute() override {
		m_vm->MakeAbsoluteMemAddr_run(m_dst_addr);
		m_vm->MakeAbsoluteMemAddr_run(m_src_addr);

		memcpy((void*)m_dst_addr.absolute_addr, (const void*)m_src_addr.absolute_addr, m_bytes);

		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
		m_vm->MakeAbsoluteMemAddr_load(m_dst_addr);
		m_vm->MakeAbsoluteMemAddr_load(m_src_addr);
	}

private:
	MemAddr m_dst_addr;
	MemAddr m_src_addr;
	int		m_bytes;
};

// 向某个内存地址写入一个常量值(integer,float)
template <typename T>
class Instruction_write_const_value : public Instruction {
public:
	// 向某个内存地址写入一个常量值
	Instruction_write_const_value(MemAddr dst_addr, T value) {
		m_dst_addr = dst_addr;
		m_value	   = value;
		m_desc	   = sprintf_to_stdstr("write value[0x%lx] %lubytes to addr[%s]", uint64_t(m_value), sizeof(m_value), dst_addr.ToString().c_str());
	}
	// 向某个内存地址写入一个常量值
	Instruction_write_const_value(const Var& var, T value) {
		m_dst_addr = var.mem_addr;
		m_value	   = value;
		m_desc	   = sprintf_to_stdstr("write value[0x%lx] %lubytes to var[%s:%s]", uint64_t(m_value), sizeof(m_value), var.var_name.c_str(), m_dst_addr.ToString().c_str());
	}

	virtual void Execute() override {
		m_vm->MakeAbsoluteMemAddr_run(m_dst_addr);

		memcpy((void*)m_dst_addr.absolute_addr, (const void*)&m_value, sizeof(m_value));
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
		m_vm->MakeAbsoluteMemAddr_load(m_dst_addr);
	}

private:
	MemAddr m_dst_addr;
	T		m_value;
};

// 向某个内存地址写入一个常量地址
class Instruction_write_addr : public Instruction {
public:
	// 向某个内存地址写入一个常量地址
	Instruction_write_addr(MemAddr dst_addr, MemAddr value_addr) {
		m_dst_addr	 = dst_addr;
		m_value_addr = value_addr;
	}
	// 向某个内存地址写入一个常量地址
	Instruction_write_addr(const Var dst_var, MemAddr value_addr) {
		m_dst_addr	 = dst_var.mem_addr;
		m_value_addr = value_addr;
		m_desc		 = sprintf_to_stdstr("write addr[%s] 8 byte to var[%s:%s]", m_value_addr.ToString().c_str(), dst_var.var_name.c_str(), m_dst_addr.ToString().c_str());
	}

	virtual void Execute() override {
		m_vm->MakeAbsoluteMemAddr_run(m_dst_addr);
		m_vm->MakeAbsoluteMemAddr_run(m_value_addr);

		memcpy((void*)m_dst_addr.absolute_addr, (const void*)&m_value_addr.absolute_addr, 8);
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
		m_vm->MakeAbsoluteMemAddr_load(m_dst_addr);
		m_vm->MakeAbsoluteMemAddr_load(m_value_addr);
	}

private:
	MemAddr m_dst_addr;
	MemAddr m_value_addr;
};

/*
 * 调用函数. 函数地址为静态固定
 *		- 保存旧sr, 设置sr为当前栈顶
 *		- 保存旧ir, 设置ir为新指令地址
 *		- 保存旧rr, 设置rr为传入的新地址
 */
class Instruction_call : public Instruction {
public:
	/*
	 * fn_id 目标函数id
	 * return_var_addr caller提供的返回值的存储位置. callee在返回前, 将返回值拷贝到这个位置. 必须是MemAddr::RELATIVE_TO_STACK_AREA
	 */
	Instruction_call(std::string fn_id, MemAddr return_var_addr) {
		assert(return_var_addr.type == MemAddr::RELATIVE_TO_STACK_AREA);
		m_fn_id							 = fn_id;
		m_is_target_instruction_addr_set = false;
		m_return_var_addr				 = return_var_addr;
	}
	Instruction_call(std::string fn_id, const Var& return_var) {
		assert(return_var.mem_addr.type == MemAddr::RELATIVE_TO_STACK_AREA);
		m_fn_id							 = fn_id;
		m_is_target_instruction_addr_set = false;
		m_return_var_addr				 = return_var.mem_addr;
		m_desc							 = sprintf_to_stdstr("call fn[%s] returned_var[%s:%s]", m_fn_id.c_str(), return_var.var_name.c_str(), m_return_var_addr.ToString().c_str());
	}

	virtual void Execute() override {
		assert(m_is_target_instruction_addr_set);
		uint64_t caller_sr = m_vm->GetReg_sr();
		uint64_t caller_ir = m_vm->GetReg_ir();
		uint64_t caller_rr = m_vm->GetReg_rr();

		uint64_t callee_sr = (uint64_t)m_vm->GetStackTop();

		uint64_t rr_absolute_addr = m_return_var_addr.relative_addr + caller_sr;

		m_vm->Push(caller_sr);
		m_vm->Push(caller_ir + 8); // 指向call的下一条指令
		m_vm->Push(caller_rr);

		m_vm->SetReg_sr(callee_sr);
		m_vm->SetReg_ir(m_target_instruction_addr.absolute_addr);
		m_vm->SetReg_rr(rr_absolute_addr);
	}
	virtual void Prepare(VM* vm) override {
		assert(m_is_target_instruction_addr_set);
		m_vm = vm;
		vm->MakeAbsoluteMemAddr_load(m_target_instruction_addr);
	}

	std::string GetFnId() const { return m_fn_id; }
	void		SetInstructionAddr(MemAddr target_instruction_addr) {
		   m_is_target_instruction_addr_set = true;
		   m_target_instruction_addr		= target_instruction_addr;
	}

private:
	std::string m_fn_id; // 调用的函数唯一id. 在函数的指令地址都确定后, 再设置真正的函数地址
	bool		m_is_target_instruction_addr_set;
	MemAddr		m_target_instruction_addr;
	MemAddr		m_return_var_addr;
};

// 退出函数
class Instruction_ret : public Instruction {
public:
	/*
	 * return_var_size 返回值大小
	 * return_var_addr callee返回的数据的地址. 将该数据拷贝到caller提供的目标地址. 必须是MemAddr::RELATIVE_TO_STACK_AREA
	 */
	Instruction_ret(uint64_t return_var_size, MemAddr return_var_addr);
	Instruction_ret();

	virtual void Execute() override;
	virtual void Prepare(VM* vm) override;

private:
	bool	 m_has_return_var;
	uint64_t m_return_var_size;
	MemAddr	 m_return_var_addr;
};

// 退出函数, 返回常量值(integer,float)
template <typename T>
class Instruction_ret_const_value : public Instruction {
public:
	/*
	 * return_var_size 返回值大小
	 * return_var_addr callee返回的数据的地址. 将该数据拷贝到caller提供的目标地址. 必须是MemAddr::RELATIVE_TO_STACK_AREA
	 */
	Instruction_ret_const_value(T return_value) {
		m_return_value = return_value;
		m_desc		   = sprintf_to_stdstr("return const value[0x%lx]", uint64_t(m_return_value));
	}

	virtual void Execute() override {
		memcpy((void*)m_vm->GetReg_rr(), (const void*)&m_return_value, sizeof(m_return_value));

		uint64_t caller_sr = *(uint64_t*)m_vm->GetReg_sr();
		uint64_t caller_ir = *((uint64_t*)m_vm->GetReg_sr() + 1);
		uint64_t caller_rr = *((uint64_t*)m_vm->GetReg_sr() + 2);

		m_vm->Pop((uint64_t)m_vm->GetStackTop() - m_vm->GetReg_sr());

		m_vm->SetReg_sr(caller_sr);
		m_vm->SetReg_ir(caller_ir);
		m_vm->SetReg_rr(caller_rr);
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	T m_return_value;
};

// 从stack上弹出一定数量的内存
class Instruction_pop : public Instruction {
public:
	Instruction_pop(int bytes) {
		m_bytes = bytes;
	}

	virtual void Execute() override {
		m_vm->Pop(m_bytes);
	}

private:
	int m_bytes;
};

// 从stack上分配内存
class Instruction_stack_alloc : public Instruction {
public:
	Instruction_stack_alloc(uint64_t bytes) {
		m_bytes = bytes;
		m_desc	= sprintf_to_stdstr("alloc %lubytes on stack", m_bytes);
	}
	virtual void Execute() override {
		m_vm->AllocStack(m_bytes);
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	int m_bytes;
};

// 执行完毕
class Instruction_exit : public Instruction {
public:
	Instruction_exit(const Var& exitcode_var) {
		m_exitcode_addr = exitcode_var.mem_addr;
		m_desc=sprintf_to_stdstr("exit exitcode[%s:%s]", exitcode_var.var_name.c_str(), m_exitcode_addr.ToString().c_str());
	}

	virtual void Execute() override {
		m_vm->MakeAbsoluteMemAddr_run(m_exitcode_addr);

		int32_t exitcode = *(int32_t*)m_exitcode_addr.absolute_addr;
		printf("exitcode=%d\n", exitcode);
		m_vm->SetReg_ir(0);
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
		m_vm->MakeAbsoluteMemAddr_load(m_exitcode_addr);
	}

private:
	MemAddr m_exitcode_addr;
};
