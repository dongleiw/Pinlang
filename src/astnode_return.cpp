#include "astnode_return.h"
#include "verify_context.h"
#include <memory>

AstNodeReturn::AstNodeReturn(AstNode* returned_expr) {
	m_returned_expr = returned_expr;
}

VerifyContextResult AstNodeReturn::Verify(VerifyContext& ctx) {
	VerifyContextParam vr_param = ctx.GetParam();
	if (m_returned_expr != nullptr) {
		// 期望表达式的结果的类型 == return的类型
		ctx.GetParam().SetResultTid(vr_param.GetReturnTid());
		m_returned_expr->Verify(ctx);
	}

	VerifyContextResult vr_result;
	return vr_result;
}
Variable* AstNodeReturn::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->SetReturnedValue(m_returned_expr->Execute(ctx));
	return nullptr;
}
