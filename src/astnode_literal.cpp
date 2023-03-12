#include "astnode_literal.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "verify_context.h"
#include <cstdint>
#include <llvm-12/llvm/ADT/APFloat.h>
#include <llvm-12/llvm/ADT/APInt.h>
#include <llvm-12/llvm/IR/Constants.h>
#include <llvm-12/llvm/IR/DerivedTypes.h>
#include <llvm-12/llvm/Support/Casting.h>

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
void AstNodeLiteral::SetNullPointer() {
	m_result_typeid = TYPE_ID_NULL;
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
	case TYPE_ID_NULL:
		if (vparam.GetResultTid() != TYPE_ID_INFER) {
			TypeInfo* ti_expect = g_typemgr.GetTypeInfo(vparam.GetResultTid());
			if (!ti_expect->IsPointer()) {
				panicf("can not cast `null` to type[%d:%s]", vparam.GetResultTid(), ti_expect->GetName().c_str());
			}
			m_result_typeid = vparam.GetResultTid();
		} else {
			panicf("bug");
		}
		return VerifyContextResult(m_result_typeid);
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
CompileResult AstNodeLiteral::Compile(CompileContext& cctx) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_result_typeid);

	llvm::Value* result = nullptr;
	if (ti->IsPointer()) {
		result = llvm::ConstantPointerNull::get((llvm::PointerType*)(ti->GetLLVMIRType(cctx)));
	}
	switch (m_result_typeid) {
	case TYPE_ID_INT8:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(8, m_value_int, true));
		break;
	case TYPE_ID_UINT8:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(8, m_value_int, false));
		break;
	case TYPE_ID_INT16:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(16, m_value_int, true));
		break;
	case TYPE_ID_UINT16:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(16, m_value_int, false));
		break;
	case TYPE_ID_INT32:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(32, m_value_int, true));
		break;
	case TYPE_ID_UINT32:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(32, m_value_int, false));
		break;
	case TYPE_ID_INT64:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(64, m_value_int, true));
		break;
	case TYPE_ID_UINT64:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(64, m_value_int, false));
		break;
	case TYPE_ID_FLOAT:
		result = llvm::ConstantFP::get(IRC, llvm::APFloat(m_value_float));
		break;
	case TYPE_ID_BOOL:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(1, m_value_bool ? 1 : 0, false));
		break;
	case TYPE_ID_STR: // str是reference type, 真实数据在堆上, 栈上只是一个指针
	{
		panicf("not implemented yet");
		break;
	}
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}

	return CompileResult().SetResult(result);
}
