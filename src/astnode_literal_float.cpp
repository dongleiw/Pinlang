#include "astnode_literal_float.h"
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

AstNodeLiteralFloat::AstNodeLiteralFloat(AstNodeType* type, double value) {
	m_type	= type;
	m_value = value;
}
VerifyContextResult AstNodeLiteralFloat::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	m_compile_to_left_value = vparam.ExpectLeftValue();

	// 将字面值转换为特定类型
	// TODO 值域校验
	TypeId target_tid = TYPE_ID_NONE;
	if (vparam.GetExpectResultTid() == TYPE_ID_INFER) {
		if (m_type == nullptr) {
			panicf("can not infer literal type");
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
	case TYPE_ID_FLOAT32:
		m_result_typeid = TYPE_ID_FLOAT32;
		return VerifyContextResult(m_result_typeid, new Variable(float(m_value)));
		break;
	case TYPE_ID_FLOAT64:
		m_result_typeid = TYPE_ID_FLOAT64;
		return VerifyContextResult(m_result_typeid, new Variable(double(m_value)));
		break;
	default:
		panicf("wrong type");
		break;
	}
}
Variable* AstNodeLiteralFloat::Execute(ExecuteContext& ctx) {
	panicf("not imp");
}
AstNodeLiteralFloat* AstNodeLiteralFloat::DeepCloneT() {
	AstNodeLiteralFloat* newone = new AstNodeLiteralFloat(m_type, m_value);
	newone->Copy(*this);

	return newone;
}
CompileResult AstNodeLiteralFloat::Compile(CompileContext& cctx) {
	TypeInfo*	ti		= g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Type* ir_type = ti->GetLLVMIRType(cctx);

	llvm::Value* result = nullptr;
	switch (m_result_typeid) {
	case TYPE_ID_FLOAT32:
		result = llvm::ConstantFP::get(IRC, llvm::APFloat(float(m_value)));
		break;
	case TYPE_ID_FLOAT64:
		result = llvm::ConstantFP::get(IRC, llvm::APFloat(m_value));
		break;
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
	if (m_compile_to_left_value) {
		llvm::Value* allocated_inst = IRB.CreateAlloca(ir_type);
		result						= IRB.CreateStore(result, allocated_inst);
	}

	return CompileResult().SetResult(result);
}
