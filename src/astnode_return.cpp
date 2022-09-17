#include "astnode_return.h"
#include "define.h"
#include "log.h"
#include "type_mgr.h"
#include "verify_context.h"

AstNodeReturn::AstNodeReturn(AstNode* returned_expr) {
	m_returned_expr = returned_expr;

	if(m_returned_expr!=nullptr){
		m_returned_expr->SetParent(this);
	}
}

VerifyContextResult AstNodeReturn::Verify(VerifyContext& ctx) {
	VerifyContextParam vr_param = ctx.GetParam();
	if (m_returned_expr != nullptr) {
		// 期望表达式的结果的类型 == return的类型
		ctx.GetParam().SetResultTid(vr_param.GetReturnTid());
		VerifyContextResult vr_result = m_returned_expr->Verify(ctx);
		if (vr_param.GetReturnTid() != TYPE_ID_INFER) {
			if (vr_result.GetResultTypeId() != vr_param.GetReturnTid()) {
				panicf("type of return value is wrong. expect[%d:%s]. give[%d:%s]",
					   vr_param.GetReturnTid(), GET_TYPENAME_C(vr_param.GetReturnTid()),
					   vr_result.GetResultTypeId(), GET_TYPENAME_C(vr_result.GetResultTypeId()));
			}
		}
	}

	VerifyContextResult vr_result;
	return vr_result;
}
Variable* AstNodeReturn::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->SetReturnedValue(m_returned_expr->Execute(ctx));
	return nullptr;
}
