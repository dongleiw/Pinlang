#include "astnode_literal.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"
#include <cstdint>

AstNodeLiteral::AstNodeLiteral(int32_t value) {
	m_value_int		= value;
	m_result_typeid = TYPE_ID_INT32;
}
AstNodeLiteral::AstNodeLiteral(int64_t value) {
	m_value_int		= value;
	m_result_typeid = TYPE_ID_INT64;
}
AstNodeLiteral::AstNodeLiteral(float value) {
	m_value_float	= value;
	m_result_typeid = TYPE_ID_FLOAT;
}
AstNodeLiteral::AstNodeLiteral(bool value) {
	m_value_bool	= value;
	m_result_typeid = TYPE_ID_BOOL;
}
AstNodeLiteral::AstNodeLiteral(std::string value) {
	m_value_str		= value;
	m_result_typeid = TYPE_ID_STR;
}
VerifyContextResult AstNodeLiteral::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	if (is_integer_type(m_result_typeid)) {
		// 将字面值转换为特定类型
		// TODO 值域校验
		TypeId target_tid = vparam.GetResultTid() == TYPE_ID_INFER ? m_result_typeid : vparam.GetResultTid();
		switch (target_tid) {
		case TYPE_ID_INT8:
			m_result_typeid = TYPE_ID_INT8;
			return VerifyContextResult(m_result_typeid, new Variable(int8_t(m_value_int)));
			break;
		case TYPE_ID_INT16:
			m_result_typeid = TYPE_ID_INT16;
			return VerifyContextResult(m_result_typeid, new Variable(int16_t(m_value_int)));
			break;
		case TYPE_ID_INT32:
			m_result_typeid = TYPE_ID_INT32;
			return VerifyContextResult(m_result_typeid, new Variable(int32_t(m_value_int)));
			break;
		case TYPE_ID_INT64:
			m_result_typeid = TYPE_ID_INT64;
			return VerifyContextResult(m_result_typeid, new Variable(int64_t(m_value_int)));
			break;
		case TYPE_ID_UINT8:
			m_result_typeid = TYPE_ID_UINT8;
			return VerifyContextResult(m_result_typeid, new Variable(uint8_t(m_value_int)));
			break;
		case TYPE_ID_UINT16:
			m_result_typeid = TYPE_ID_UINT16;
			return VerifyContextResult(m_result_typeid, new Variable(uint16_t(m_value_int)));
			break;
		case TYPE_ID_UINT32:
			m_result_typeid = TYPE_ID_UINT32;
			return VerifyContextResult(m_result_typeid, new Variable(uint32_t(m_value_int)));
			break;
		case TYPE_ID_UINT64:
			m_result_typeid = TYPE_ID_UINT64;
			return VerifyContextResult(m_result_typeid, new Variable(uint64_t(m_value_int)));
			break;
		default:
			panicf("wrong type");
			break;
		}
	}
	switch (m_result_typeid) {
	case TYPE_ID_FLOAT:
		return VerifyContextResult(m_result_typeid, new Variable(m_value_float));
		break;
	case TYPE_ID_BOOL:
		return VerifyContextResult(m_result_typeid, new Variable(m_value_bool));
		break;
	case TYPE_ID_STR:
		return VerifyContextResult(m_result_typeid, new Variable(m_value_str));
		break;
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
}
Variable* AstNodeLiteral::Execute(ExecuteContext& ctx) {
	switch (m_result_typeid) {
	case TYPE_ID_INT8:
		return new Variable(int8_t(this->m_value_int));
		break;
	case TYPE_ID_INT16:
		return new Variable(int16_t(this->m_value_int));
		break;
	case TYPE_ID_INT32:
		return new Variable(int32_t(this->m_value_int));
		break;
	case TYPE_ID_INT64:
		return new Variable(int64_t(this->m_value_int));
		break;
	case TYPE_ID_UINT8:
		return new Variable(uint8_t(this->m_value_int));
		break;
	case TYPE_ID_UINT16:
		return new Variable(uint16_t(this->m_value_int));
		break;
	case TYPE_ID_UINT32:
		return new Variable(uint32_t(this->m_value_int));
		break;
	case TYPE_ID_UINT64:
		return new Variable(uint64_t(this->m_value_int));
		break;

	case TYPE_ID_FLOAT:
		return new Variable(this->m_value_float);
		break;
	case TYPE_ID_BOOL:
		return new Variable(this->m_value_bool);
		break;
	case TYPE_ID_STR:
		return new Variable(this->m_value_str);
		break;
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
}
AstNodeLiteral* AstNodeLiteral::DeepCloneT() {
	AstNodeLiteral* newone = new AstNodeLiteral();

	*newone = *this;

	return newone;
}
void AstNodeLiteral::CastToInt64() {
	assert(m_result_typeid == TYPE_ID_INT32);
	m_result_typeid = TYPE_ID_INT64;
}
void AstNodeLiteral::Compile(VM& vm, FnInstructionMaker& maker, MemAddr& target_addr) {
	switch (m_result_typeid) {
	case TYPE_ID_INT8:
	case TYPE_ID_UINT8:
		maker.AddInstruction(new Instruction_write_const_value<uint8_t>(target_addr, uint8_t(m_value_int)));
		break;
	case TYPE_ID_INT16:
	case TYPE_ID_UINT16:
		maker.AddInstruction(new Instruction_write_const_value<uint16_t>(target_addr, uint16_t(m_value_int)));
		break;
	case TYPE_ID_INT32:
	case TYPE_ID_UINT32:
		maker.AddInstruction(new Instruction_write_const_value<uint32_t>(target_addr, uint32_t(m_value_int)));
		break;
	case TYPE_ID_INT64:
	case TYPE_ID_UINT64:
		maker.AddInstruction(new Instruction_write_const_value<uint64_t>(target_addr, uint64_t(m_value_int)));
		break;
	case TYPE_ID_FLOAT:
		maker.AddInstruction(new Instruction_write_const_value<float>(target_addr, m_value_float));
		break;
	case TYPE_ID_BOOL:
		maker.AddInstruction(new Instruction_write_const_value<bool>(target_addr, m_value_bool));
		break;
	case TYPE_ID_STR: // str是reference type, 真实数据在堆上, 栈上只是一个指针
	{
		MemAddr addr = vm.AddStaticData((const void*)m_value_str.c_str(), m_value_str.size());
		maker.AddInstruction(new Instruction_write_addr(target_addr, addr));
		break;
	}
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
}
/*void AstNodeLiteral::Compile(VM& vm, FnInstructionMaker& maker, Var& returned_var) {
	switch (m_result_typeid) {
	case TYPE_ID_INT8:
	case TYPE_ID_UINT8:
	{
		uint8_t v			  = uint8_t(m_value_int);
		MemAddr addr		  = vm.AddStaticData((const void*)&v, sizeof(v));
		returned_var.mem_addr = addr;
		returned_var.mem_size = 1;
		break;
	}
	case TYPE_ID_INT16:
	case TYPE_ID_UINT16:
	{
		uint16_t v			  = uint16_t(m_value_int);
		MemAddr	 addr		  = vm.AddStaticData((const void*)&v, sizeof(v));
		returned_var.mem_addr = addr;
		returned_var.mem_size = 2;
		break;
	}
	case TYPE_ID_INT32:
	case TYPE_ID_UINT32:
	{
		uint32_t v			  = uint32_t(m_value_int);
		MemAddr	 addr		  = vm.AddStaticData((const void*)&v, sizeof(v));
		returned_var.mem_addr = addr;
		returned_var.mem_size = 4;
		break;
	} break;
	case TYPE_ID_INT64:
	case TYPE_ID_UINT64:
	{
		uint64_t v			  = uint64_t(m_value_int);
		MemAddr	 addr		  = vm.AddStaticData((const void*)&v, sizeof(v));
		returned_var.mem_addr = addr;
		returned_var.mem_size = 8;
		break;
	}
	case TYPE_ID_FLOAT:
	{
		MemAddr addr		  = vm.AddStaticData((const void*)&m_value_float, sizeof(m_value_float));
		returned_var.mem_addr = addr;
		returned_var.mem_size = 4;
		break;
	}
	case TYPE_ID_BOOL:
	{
		MemAddr addr		  = vm.AddStaticData((const void*)&m_value_bool, sizeof(m_value_bool));
		returned_var.mem_addr = addr;
		returned_var.mem_size = 1;
		break;
	}
	case TYPE_ID_STR: // str是reference type, 真实数据在堆上, 栈上只是一个指针
	{
		MemAddr addr		  = vm.AddStaticData((const void*)m_value_str.c_str(), m_value_str.size());
		returned_var.mem_addr = addr;
		returned_var.mem_size = 8;
		break;
	}
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
}*/
