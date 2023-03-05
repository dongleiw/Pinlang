#include "astnode_return.h"
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
	m_is_exit_node = true;
	if (m_returned_expr != nullptr) {
		// 期望表达式的结果的类型 == return的类型
		VerifyContextParam vparam_return;
		vparam_return.SetReturnTid(vparam.GetReturnTid());
		VerifyContextResult vr_result = m_returned_expr->Verify(ctx, vparam_return);
		if (vparam.GetReturnTid() != TYPE_ID_INFER) {
			if (vr_result.GetResultTypeId() != vparam.GetReturnTid()) {
				panicf("type of return value is wrong. expect[%d:%s]. give[%d:%s]",
					   vparam.GetReturnTid(), GET_TYPENAME_C(vparam.GetReturnTid()),
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

	if (m_returned_expr != nullptr)
		newone->m_returned_expr = m_returned_expr->DeepClone();

	return newone;
}
llvm::Value* AstNodeReturn::Compile(CompileContext& cctx) {
	if (m_returned_expr == nullptr) {
		IRB.CreateRetVoid();
	} else {
		TypeInfo*	 ti_return = g_typemgr.GetTypeInfo(m_returned_expr_tid);
		llvm::Value* ret_value = m_returned_expr->Compile(cctx);

		assert(ret_value->getType() == ti_return->GetLLVMIRType(cctx));
		IRB.CreateRet(ret_value);
	}
	return nullptr;
}
