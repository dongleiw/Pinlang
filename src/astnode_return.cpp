#include "astnode_return.h"
#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/Value.h>

AstNodeReturn::AstNodeReturn(AstNode* returned_expr) {
	m_returned_expr		= returned_expr;
	m_returned_expr_tid = TYPE_ID_NONE;

	if (m_returned_expr != nullptr) {
		m_returned_expr->SetParent(this);
	}
}

VerifyContextResult AstNodeReturn::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	m_is_exit_node = true;
	if (m_returned_expr != nullptr) {
		// 期望表达式的结果的类型 == return的类型
		VerifyContextResult vr_result = m_returned_expr->Verify(ctx, VerifyContextParam().SetExpectResultTid(vparam.GetExpectReturnTid()));
		if (vparam.GetExpectReturnTid() != TYPE_ID_INFER) {
			if (vr_result.GetResultTypeId() != vparam.GetExpectReturnTid()) {
				panicf("type of return value is wrong. expect[%d:%s]. give[%d:%s]",
					   vparam.GetExpectReturnTid(), GET_TYPENAME_C(vparam.GetExpectReturnTid()),
					   vr_result.GetResultTypeId(), GET_TYPENAME_C(vr_result.GetResultTypeId()));
			}
		}
		m_returned_expr_tid	   = vr_result.GetResultTypeId();
		m_returned_expr_is_tmp = vr_result.IsTmp();
	}

	VerifyContextResult vr_result;
	return vr_result;
}
Variable* AstNodeReturn::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->SetReturnedValue(m_returned_expr->Execute(ctx));
	return nullptr;
}
AstNodeReturn* AstNodeReturn::DeepCloneT() {
	AstNodeReturn* newone = new AstNodeReturn();
	newone->Copy(*this);

	if (m_returned_expr != nullptr)
		newone->m_returned_expr = m_returned_expr->DeepClone();

	return newone;
}
CompileResult AstNodeReturn::Compile(CompileContext& cctx) {
	if (m_returned_expr == nullptr) {
		IRB.CreateRetVoid();
	} else {
		TypeInfo*	  ti_return	   = g_typemgr.GetTypeInfo(m_returned_expr_tid);
		CompileResult cr_ret_value = m_returned_expr->Compile(cctx);

		assert(cr_ret_value.GetResult()->getType() == ti_return->GetLLVMIRType(cctx));
		IRB.CreateRet(cr_ret_value.GetResult());
	}
	return CompileResult();
}
