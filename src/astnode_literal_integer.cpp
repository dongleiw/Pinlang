#include "astnode_literal_integer.h"
#include "astnode.h"
#include "astnode_type.h"
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
#include <llvm-12/llvm/IR/Constant.h>
#include <llvm-12/llvm/IR/Constants.h>
#include <llvm-12/llvm/IR/DerivedTypes.h>
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/Support/Casting.h>
#include <vector>

AstNodeLiteralInteger::AstNodeLiteralInteger(AstNodeType* type, int64_t value) {
	m_type	= type;
	m_value = value;
}
VerifyContextResult AstNodeLiteralInteger::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	m_compile_to_left_value = vparam.ExpectLeftValue();

	// 将字面值转换为特定类型
	// TODO 值域校验
	TypeId target_tid = TYPE_ID_NONE;
	if (vparam.GetExpectResultTid() == TYPE_ID_INFER) {
		if (m_type == nullptr) {
			// 缺省情况下, 常量整形类型为i32.
			// 如果根据常量整形的值自动升级为i64, 则会根据值的变化而影响程序的行为. 这会降低程序可读性
			target_tid = TYPE_ID_INT32;
		} else {
			VerifyContextResult vr_type = m_type->Verify(ctx, VerifyContextParam());
			target_tid					= vr_type.GetResultTypeId();
		}
	} else {
		if (m_type == nullptr) {
			target_tid = vparam.GetExpectResultTid();
		} else {
			VerifyContextResult vr_type = m_type->Verify(ctx, VerifyContextParam());
			if (vr_type.GetResultTypeId() != vparam.GetExpectResultTid()) {
				panicf("expect type[%d:%s] != explicit type[%d:%s]", vparam.GetExpectResultTid(), GET_TYPENAME_C(vparam.GetExpectResultTid()),
					   vr_type.GetResultTypeId(), GET_TYPENAME_C(vr_type.GetResultTypeId()));
			}
			target_tid = vr_type.GetResultTypeId();
		}
	}

	switch (target_tid) {
	case TYPE_ID_INT8:
		m_result_typeid = TYPE_ID_INT8;
		return VerifyContextResult(m_result_typeid, new Variable(int8_t(m_value)));
		break;
	case TYPE_ID_INT16:
		m_result_typeid = TYPE_ID_INT16;
		return VerifyContextResult(m_result_typeid, new Variable(int16_t(m_value)));
		break;
	case TYPE_ID_INT32:
		m_result_typeid = TYPE_ID_INT32;
		return VerifyContextResult(m_result_typeid, new Variable(int32_t(m_value)));
		break;
	case TYPE_ID_INT64:
		m_result_typeid = TYPE_ID_INT64;
		return VerifyContextResult(m_result_typeid, new Variable(int64_t(m_value)));
		break;
	case TYPE_ID_UINT8:
		m_result_typeid = TYPE_ID_UINT8;
		return VerifyContextResult(m_result_typeid, new Variable(uint8_t(m_value)));
		break;
	case TYPE_ID_UINT16:
		m_result_typeid = TYPE_ID_UINT16;
		return VerifyContextResult(m_result_typeid, new Variable(uint16_t(m_value)));
		break;
	case TYPE_ID_UINT32:
		m_result_typeid = TYPE_ID_UINT32;
		return VerifyContextResult(m_result_typeid, new Variable(uint32_t(m_value)));
		break;
	case TYPE_ID_UINT64:
		m_result_typeid = TYPE_ID_UINT64;
		return VerifyContextResult(m_result_typeid, new Variable(uint64_t(m_value)));
		break;
	default:
		panicf("wrong type[%d]", target_tid);
		break;
	}
}
Variable* AstNodeLiteralInteger::Execute(ExecuteContext& ctx) {
	panicf("not imp");
}
AstNodeLiteralInteger* AstNodeLiteralInteger::DeepCloneT() {
	AstNodeLiteralInteger* newone = new AstNodeLiteralInteger(m_type, m_value);
	newone->Copy(*this);

	return newone;
}
void AstNodeLiteralInteger::CastToInt64() {
	assert(m_result_typeid == TYPE_ID_INT32);
	m_result_typeid = TYPE_ID_INT64;
}
CompileResult AstNodeLiteralInteger::Compile(CompileContext& cctx) {
	TypeInfo*	ti		= g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Type* ir_type = ti->GetLLVMIRType(cctx);

	llvm::Value* result = nullptr;
	switch (m_result_typeid) {
	case TYPE_ID_INT8:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(8, m_value, true));
		break;
	case TYPE_ID_UINT8:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(8, m_value, false));
		break;
	case TYPE_ID_INT16:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(16, m_value, true));
		break;
	case TYPE_ID_UINT16:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(16, m_value, false));
		break;
	case TYPE_ID_INT32:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(32, m_value, true));
		break;
	case TYPE_ID_UINT32:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(32, m_value, false));
		break;
	case TYPE_ID_INT64:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(64, m_value, true));
		break;
	case TYPE_ID_UINT64:
		result = llvm::ConstantInt::get(IRC, llvm::APInt(64, m_value, false));
		break;
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
	if (m_compile_to_left_value) {
		llvm::Value* allocated_inst = IRB.CreateAlloca(ir_type);
		IRB.CreateStore(result, allocated_inst);
		result = allocated_inst;
	}

	return CompileResult().SetResult(result);
}
