#pragma once

#include "log.h"
#include "utils.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <vector>

class VM;
class FnInstructionMaker;

typedef int RegisterId;

const RegisterId REGISTER_ID_STACK_FRAME = 0;

struct MemAddr {
	enum AddrType {
		RELATIVE_TO_STACK_AREA,		  // 相对当前sr
		RELATIVE_TO_STATIC_AREA,	  // 相对static area的起始地址
		RELATIVE_TO_INSTRUCTION_AREA, // 相对instruction area的起始地址
		PTR_RELATIVE_TO_STACK_AREA,	  // 在`RELATIVE_TO_STACK_AREA`位置读取一个地址
		STATIC_FN_ID,				  // 是静态函数, 静态函数在编译阶段确定函数的指令相对地址, 保存函数名字.
	};
	AddrType	type;
	int64_t		relative_addr;
	uint64_t	absolute_addr;
	std::string fn_id;
	std::string tmp_name;
	bool		is_pointer;

	MemAddr(AddrType a_type, int64_t a_relative_addr) : type(a_type), relative_addr(a_relative_addr), absolute_addr(0), is_pointer(false) {
	}
	MemAddr() : type(RELATIVE_TO_STACK_AREA), relative_addr(0), absolute_addr(0), is_pointer(false) {
	}
	MemAddr(std::string fn_id) : type(STATIC_FN_ID), fn_id(fn_id), is_pointer(false) {
	}

	std::string ToString(FnInstructionMaker& maker) const;
};

//class CompileResult {
//public:
//	CompileResult() : m_unset(true) {
//	}
//	CompileResult(RegisterId rid, bool is_value, std::string stack_var_name) {
//		m_unset			 = false;
//		m_rid			 = rid;
//		m_is_value		 = is_value;
//		m_stack_var_name = stack_var_name;
//	}
//	CompileResult(std::string fn_id) {
//		m_unset = false;
//		m_fn_id = fn_id;
//	}
//
//	bool		IsFnId() const;
//	std::string GetFnId() const;
//	RegisterId	GetRegisterId() const;
//	bool		IsValue() const;
//	std::string GetStackVarName() const;
//
//private:
//	bool		m_unset;
//	std::string m_fn_id; // 是一个静态函数. 这里记录函数的id. 此时rid未定义
//	RegisterId	m_rid;
//	bool		m_is_value;		  // 寄存器中是一个指向实际数据的指针, 还是一个值
//	std::string m_stack_var_name; // 如果申请了临时栈内存, 这里记录名字. 否则为空
//};

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
/*
 * 指令注释
 */
class InstructionComment {
public:
	InstructionComment() {
	}
	InstructionComment(std::string content) {
		m_content = std::string("  // ") + content;
	}
	InstructionComment(int align, std::string content) {
		std::string prefix = "";
		for (int i = 0; i < align; i++) {
			prefix += "/";
		}
		m_content = prefix + "// " + content;
	}
	void			   Merge(const InstructionComment& another);
	const std::string& GetContent() const { return m_content; }

protected:
	std::string m_content;
};

struct Var {
	std::string var_name;
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
		m_fn_id				= fn_id;
		m_cur_offset		= START_OFFSET;
		m_max_offset		= m_cur_offset;
		m_tmp_var_name_seed = 1;
	}

	// 一个变量开始生命周期
	Var VarBegin(std::string var_name, uint64_t size);
	Var TmpVarBegin(std::string prefix, uint64_t size);
	// 一个变量结束声明周期. 之后该变量的栈内存就可以释放掉给后续变量使用了
	void VarEnd(std::string var_name);

	const Var GetVar(std::string var_name) const;
	bool	  HasVar(std::string var_name) const;
	const Var GetVarByStackOffset(int64_t stack_offset) const;

	// 设置函数的参数. mem_addr为相对callee的stack-frame
	void SetArg(std::string var_name, uint64_t mem_size, MemAddr mem_addr);

	void							AddInstruction(Instruction* instruction);
	bool							HasInstruction(const Instruction* instruction) const;
	const std::string&				GetFnId() const { return m_fn_id; }
	const std::vector<Instruction*> GetInstructions() const { return m_instructions; }

	// 函数指令已经生成添加完毕.
	// 生成第一条指令. 一次性分配所有栈内存
	void Finish();

	void AddComment(InstructionComment comment);
	void InsertComments(size_t fn_start_idx, std::map<size_t, InstructionComment>& comments) const;
	void SetFnComment(InstructionComment comment);

private:
	std::string				  m_fn_id;
	std::vector<Instruction*> m_instructions;
	std::vector<Var>		  m_vars;		// 分配的变量列表. 按照栈增长的方向依次排序
	uint64_t				  m_cur_offset; // 当前已分配的栈内存的偏移
	uint64_t				  m_max_offset; // 记录分配栈内存过程中的最大偏移. 在函数开始位置一次性分配完.
	int						  m_tmp_var_name_seed;

	std::map<size_t, InstructionComment> m_comments;
	InstructionComment					 m_fn_comment;
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
private:
	struct Register {
		uint64_t value;
		bool	 is_empty;
	};

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

	uint64_t GetReg_rr() const { return m_reg_return_mem_address; }
	void	 SetReg_rr(uint64_t rr) { m_reg_return_mem_address = rr; }

	RegisterId AllocGeneralRegister();				   // 分配通用寄存器, 如果没有空闲的, 则panic
	void	   ReleaseGeneralRegister(RegisterId rid); // 回收通用寄存器

	template <typename T>
	T GetRegister(RegisterId rid) {
		assert(sizeof(T) <= sizeof(Register::value));
		return *(T*)&(m_registers.at(rid).value);
	}
	const void* GetRegisterAddr(RegisterId rid);
	void		SetRegister(RegisterId rid, const void* ptr, size_t bytes);

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

	void AddFn(const FnInstructionMaker& maker);
	// 指令设置过程中, 函数的调用只是设置了fnid, 需要刷新函数的地址
	void RefreshFnAddr();

	uint64_t GetInstructionAreaStartAddr() const {
		return (uint64_t) & (m_instructions[0]);
	}

public: // test
	void test1();

private:
	void prepare_all();

private:
	std::vector<Instruction*>			 m_instructions;
	std::map<std::string, uint64_t>		 m_fn_id_2_instruction_addr_offset; // 函数id到函数的指令起始地址(相对instruction area的偏移)的映射关系
	std::map<size_t, InstructionComment> m_comments;

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

	/*
	 * 寄存器统一都是uint64_t. 
	 * 这会带来一个问题, 如何在uint64_t中保存其他类型的值. 比如uint32_t, float, double
	 *		约定: 
	 *			如果存储1byte, 则存储在uin64_t的第1个字节
	 *			如果存储2byte, 则存储在uin64_t的前2个字节
	 *			如果存储4byte, 则存储在uin64_t的前4个字节
	 *			如果存储8byte, 则使用所有字节
	 */
	std::vector<Register> m_registers;
};

/*
 * 将左寄存器中的值(或者是地址对应的值)和右寄存器中的值(或者是地址对应的值)相加
 * T:
 *		值的类型
 * RESULT_IS_VALUE: 
 *		true: 结果存储在寄存器中
 *		false: result寄存器中保存了一个内存地址, 结果写入该内存地址指向的内存
 * LEFT_IS_VALUE: 
 *		true: left寄存器中是值
 *		false: left寄存器中保存了一个内存地址, 值保存在该内存地址指向的内存中
 * RIGHT_IS_VALUE: 
 *		true: right寄存器中是值
 *		false: right寄存器中保存了一个内存地址, 值保存在该内存地址指向的内存中
 */
template <typename T, bool RESULT_IS_VALUE, bool LEFT_IS_VALUE, bool RIGHT_IS_VALUE>
class Instruction_add : public Instruction {
public:
	Instruction_add(FnInstructionMaker& maker, RegisterId r_result, RegisterId r_left, RegisterId r_right) {
		m_r_result = r_result;
		m_r_left   = r_left;
		m_r_right  = r_right;

		m_desc = sprintf_to_stdstr("add r-left[%d:%s] r-right[%d:%s] r-result[%d:%s] type[%s]",
								   r_left, LEFT_IS_VALUE ? "value" : "ptr",
								   r_right, RIGHT_IS_VALUE ? "value" : "ptr",
								   r_result, RESULT_IS_VALUE ? "value" : "ptr",
								   get_type_name<T>());
	}

	virtual void Execute() override {
		T left_value, right_value, result_value;

		if (LEFT_IS_VALUE) {
			// left寄存器中保存了值
			left_value = m_vm->GetRegister<T>(m_r_left);
		} else {
			// left寄存器中保存了值的内存地址
			left_value = *(T*)m_vm->GetRegister<uint64_t>(m_r_left);
		}

		if (RIGHT_IS_VALUE) {
			// right寄存器中保存了值
			right_value = m_vm->GetRegister<T>(m_r_right);
		} else {
			// right寄存器中保存了值的内存地址
			right_value = *(T*)m_vm->GetRegister<uint64_t>(m_r_right);
		}

		result_value = left_value + right_value;

		if (RESULT_IS_VALUE) {
			m_vm->SetRegister(m_r_result, &result_value, sizeof(result_value));
		} else {
			// result寄存器中保存的是内存地址, 将结果拷贝过去
			uint64_t result_addr = m_vm->GetRegister<uint64_t>(m_r_result);
			memcpy((void*)result_addr, &result_value, sizeof(result_value));
		}

		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	RegisterId m_r_result;
	RegisterId m_r_left;
	RegisterId m_r_right;
};

/*
 * 将左寄存器中的值(或者是地址对应的值)和常量相加
 * T:
 *		值的类型. 
 * RESULT_IS_VALUE: 
 *		true: 结果存储在寄存器中
 *		false: result寄存器中保存了一个内存地址, 结果写入该内存地址指向的内存
 * LEFT_IS_VALUE: 
 *		true: left寄存器中是值
 *		false: left寄存器中保存了一个内存地址, 值保存在该内存地址指向的内存中
 */
template <typename T, bool RESULT_IS_VALUE, bool LEFT_IS_VALUE>
class Instruction_add_const : public Instruction {
public:
	Instruction_add_const(FnInstructionMaker& maker, RegisterId r_result, RegisterId r_left, T const_value, std::string comment) {
		init(maker, r_result, r_left, const_value, comment);
	}
	Instruction_add_const(FnInstructionMaker& maker, RegisterId r_result, RegisterId r_left, T const_value) {
		init(maker, r_result, r_left, const_value, std::string(""));
	}

	virtual void Execute() override {
		T result;
		if (LEFT_IS_VALUE) {
			// left寄存器中保存了值, 将其强行转换为T类型
			result = m_vm->GetRegister<T>(m_r_left) + m_r_right_const_value;
		} else {
			// left寄存器中保存了值的内存地址
			result = *(T*)m_vm->GetRegister<uint64_t>(m_r_left) + m_r_right_const_value;
		}

		if (RESULT_IS_VALUE) {
			m_vm->SetRegister(m_r_result, &result, sizeof(result));
		} else {
			// result寄存器中保存的是内存地址, 将结果拷贝过去
			uint64_t result_addr = m_vm->GetRegister<uint64_t>(m_r_result);
			memcpy((void*)result_addr, &result, sizeof(result));
		}

		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	void init(FnInstructionMaker& maker, RegisterId r_result, RegisterId r_left, T const_value, std::string comment) {
		m_r_result			  = r_result;
		m_r_left			  = r_left;
		m_r_right_const_value = const_value;

		m_desc = sprintf_to_stdstr("add const: r-left[%d:%s] r-right_const_value[%s] r-result[%d:%s] ",
								   r_left, LEFT_IS_VALUE ? "value" : "ptr",
								   to_str(m_r_right_const_value).c_str(),
								   r_result, RESULT_IS_VALUE ? "value" : "ptr");
		if (!comment.empty()) {
			m_desc += "// " + comment;
		}
	}

private:
	RegisterId m_r_result;
	RegisterId m_r_left;
	T		   m_r_right_const_value;
};

/*
 * 将left寄存器中的值(或者是地址指向的值) 和常量相乘, 结果存储到result寄存器(或者是地址指向的内存)
 * T uint(8/16/32/64)_t int(8/16/32/64_t  float double 
 */
template <typename T, bool RESULT_IS_VALUE, bool LEFT_IS_VALUE>
class Instruction_mul_const : public Instruction {
public:
	Instruction_mul_const(FnInstructionMaker& maker, RegisterId register_result, RegisterId register_left, T right_value) {
		assert(sizeof(T) <= 8);
		m_register_result = register_result;
		m_register_left	  = register_left;
		m_right_value	  = right_value;
		m_desc			  = sprintf_to_stdstr("mul_const: left-register[%d:%s] * right-value[%s] => result-register[%d:%s]",
									  register_left, LEFT_IS_VALUE ? "value" : "ptr",
									  to_str(m_right_value).c_str(),
									  register_result, RESULT_IS_VALUE ? "value" : "ptr");
	}

	virtual void Execute() override {
		T result_value;
		if (LEFT_IS_VALUE) {
			result_value = m_vm->GetRegister<T>(m_register_left) * m_right_value;
		} else {
			result_value = *(T*)m_vm->GetRegister<uint64_t>(m_register_left) * m_right_value;
		}

		if (RESULT_IS_VALUE) {
			m_vm->SetRegister(m_register_result, (const void*)&result_value, sizeof(result_value));
		} else {
			uint64_t result_addr = m_vm->GetRegister<uint64_t>(m_register_result);
			memcpy((void*)result_addr, (const void*)&result_value, sizeof(result_value));
		}

		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	RegisterId m_register_result;
	RegisterId m_register_left;
	T		   m_right_value;
};

/*
 * 将src寄存器中的地址指向的值拷贝到dst寄存器中的地址指向的内存
 * register_dst 存放目标内存的地址
 * register_src 存放源内存的地址
 */
class Instruction_memcpy : public Instruction {
public:
	Instruction_memcpy(FnInstructionMaker& maker, RegisterId register_dst, RegisterId register_src, uint64_t bytes) {
		init(maker, register_dst, register_src, bytes, "");
	}
	Instruction_memcpy(FnInstructionMaker& maker, RegisterId register_dst, RegisterId register_src, uint64_t bytes, std::string comment) {
		init(maker, register_dst, register_src, bytes, comment);
	}

	virtual void Execute() override {
		uint64_t src_addr = m_vm->GetRegister<uint64_t>(m_register_src);
		uint64_t dst_addr = m_vm->GetRegister<uint64_t>(m_register_dst);
		memcpy((void*)dst_addr, (const void*)src_addr, m_bytes);
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	void init(FnInstructionMaker& maker, RegisterId register_dst, RegisterId register_src, uint64_t bytes, std::string comment) {
		m_register_dst = register_dst;
		m_register_src = register_src;
		m_bytes		   = bytes;
		m_desc		   = sprintf_to_stdstr("copy %lu bytes from register[%d] to register[%d]", bytes, m_register_src, m_register_dst);

		if (!comment.empty()) {
			m_desc += "// " + comment;
		}
	}

private:
	RegisterId m_register_dst;
	RegisterId m_register_src;
	uint64_t   m_bytes;
};

// 向某个内存地址写入一个常量值(integer,float)
template <typename T>
class Instruction_write_const_value : public Instruction {
public:
	Instruction_write_const_value(FnInstructionMaker& maker, RegisterId rid, T value) {
		m_rid	= rid;
		m_value = value;
		m_desc	= sprintf_to_stdstr("write_const_value rid_addr[%d] type[%s] value[%s]", rid, get_type_name<T>(), to_str(value).c_str());
	}

	virtual void Execute() override {
		uint64_t target_addr = m_vm->GetRegister<uint64_t>(m_rid);
		memcpy((void*)target_addr, (const void*)&m_value, sizeof(m_value));
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	RegisterId m_rid;
	T		   m_value;
};

// 将一个常量值加载到某个寄存器
template <typename T>
class Instruction_load_register_const : public Instruction {
public:
	Instruction_load_register_const(FnInstructionMaker& maker, RegisterId rid, T value) {
		init(maker, rid, value, std::string(""));
	}
	Instruction_load_register_const(FnInstructionMaker& maker, RegisterId rid, T value, std::string comment) {
		init(maker, rid, value, comment);
	}

	virtual void Execute() override {
		m_vm->SetRegister(m_rid, &m_const_value, sizeof(m_const_value));
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	void init(FnInstructionMaker& maker, RegisterId rid, T value, std::string comment) {
		m_rid		  = rid;
		m_const_value = value;
		m_desc		  = sprintf_to_stdstr("load_register_const rid[%d] type[%s] value[%s] // %s", rid, get_type_name<T>(), to_str(m_const_value).c_str(), comment.c_str());
	}

private:
	RegisterId m_rid;
	T		   m_const_value;
};

// 将value寄存器中的值保存到addr寄存器中的内存地址指向的内存块
class Instruction_store_register : public Instruction {
public:
	Instruction_store_register(FnInstructionMaker& maker, RegisterId register_addr, RegisterId register_value, int bytes) {
		init(maker, register_addr, register_value, bytes, "");
	}
	Instruction_store_register(FnInstructionMaker& maker, RegisterId register_addr, RegisterId register_value, int bytes, std::string comment) {
		init(maker, register_addr, register_value, bytes, comment);
	}

	virtual void Execute() override {
		const void* value_ptr = m_vm->GetRegisterAddr(m_rid_value);
		uint64_t	addr	  = m_vm->GetRegister<uint64_t>(m_rid_addr);
		memcpy((void*)addr, value_ptr, m_bytes);
		m_vm->Inc_ir();
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	void init(FnInstructionMaker& maker, RegisterId register_addr, RegisterId register_value, int bytes, std::string comment) {
		m_rid_addr	= register_addr;
		m_rid_value = register_value;
		m_bytes		= bytes;
		m_desc		= sprintf_to_stdstr("store_register rid_addr[%d] rid_value[%d] bytes[%d]", m_rid_addr, m_rid_value, bytes);
		if (!comment.empty()) {
			m_desc += "// " + comment;
		}
	}

private:
	RegisterId m_rid_addr;
	RegisterId m_rid_value;
	int		   m_bytes;
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
	 * return_var_offset 保存返回值的内存地址(相对caller的栈帧的偏移地址)
	 */
	Instruction_call(FnInstructionMaker& maker, std::string fn_id, int64_t return_var_offset);

	virtual void Execute() override;
	virtual void Prepare(VM* vm) override;

	std::string GetFnId() const { return m_fn_id; }

	void SetFnInstructionAddrOffset(uint64_t fn_instruction_addr_offset) {
		m_is_fn_instruction_addr_set = true;
		m_fn_instruction_addr_offset = fn_instruction_addr_offset;
	}

private:
	std::string m_fn_id; // 调用的函数唯一id. 在函数的指令地址都确定后, 再设置真正的函数地址
	bool		m_is_fn_instruction_addr_set;
	uint64_t	m_fn_instruction_addr_offset;
	uint64_t	m_fn_instruction_addr;
	int64_t		m_return_var_offset;
};

// 退出函数
template <bool REGISTER_IS_VALUE>
class Instruction_ret : public Instruction {
public:
	/*
	 * rid_returned 
	 *		如果REGISTER_IS_VALUE, 则rid_returned中存储了返回的数据. 否则rid_returned中存储了返回的数据的内存地址
	 * return_var_size 返回值大小
	 */
	Instruction_ret(FnInstructionMaker& maker, RegisterId rid_returned, uint64_t returned_value_size) {
		m_has_return_var	  = true;
		m_rid_returned		  = rid_returned;
		m_returned_value_size = returned_value_size;
		m_desc				  = sprintf_to_stdstr("ret register_returned[%d:%s] %lu byte", m_rid_returned, REGISTER_IS_VALUE ? "value" : "ptr", m_returned_value_size);
	}
	Instruction_ret() {
		m_has_return_var = false;
		m_desc			 = sprintf_to_stdstr("ret");
	}

	virtual void Execute() override {
		if (m_has_return_var) {
			const void* data_ptr = nullptr;
			if (REGISTER_IS_VALUE) {
				data_ptr = m_vm->GetRegisterAddr(m_rid_returned);
			} else {
				data_ptr = (const void*)m_vm->GetRegister<uint64_t>(m_rid_returned);
			}
			memcpy((void*)m_vm->GetReg_rr(), data_ptr, m_returned_value_size);
		}

		uint64_t caller_sr = *(uint64_t*)m_vm->GetRegister<uint64_t>(REGISTER_ID_STACK_FRAME);
		uint64_t caller_ir = *((uint64_t*)m_vm->GetRegister<uint64_t>(REGISTER_ID_STACK_FRAME) + 1);
		uint64_t caller_rr = *((uint64_t*)m_vm->GetRegister<uint64_t>(REGISTER_ID_STACK_FRAME) + 2);

		m_vm->Pop((uint64_t)m_vm->GetStackTop() - m_vm->GetRegister<uint64_t>(REGISTER_ID_STACK_FRAME));

		m_vm->SetRegister(REGISTER_ID_STACK_FRAME, &caller_sr, sizeof(caller_sr));
		m_vm->SetReg_ir(caller_ir);
		m_vm->SetReg_rr(caller_rr);
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	bool	   m_has_return_var;
	RegisterId m_rid_returned;
	uint64_t   m_returned_value_size;
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
	Instruction_exit(FnInstructionMaker& maker, RegisterId register_exitcode_var_addr) {
		m_register_exitcode_var_addr = register_exitcode_var_addr;
		m_desc						 = sprintf_to_stdstr("exit register_exitcode_var_addr[%d]", register_exitcode_var_addr);
	}

	virtual void Execute() override {
		uint64_t exitcode_var_addr = m_vm->GetRegister<uint64_t>(m_register_exitcode_var_addr);
		int32_t	 exitcode		   = *(int32_t*)exitcode_var_addr;
		printf("exitcode=%d\n", exitcode);
		m_vm->SetReg_ir(0);
	}
	virtual void Prepare(VM* vm) override {
		m_vm = vm;
	}

private:
	RegisterId m_register_exitcode_var_addr;
};
