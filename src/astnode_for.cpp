#include "astnode_for.h"
#include "define.h"
#include "log.h"
#include "variable_table.h"
#include "verify_context.h"

#include <cassert>

VerifyContextResult AstNodeFor::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify for");

	if (m_init_expr != nullptr) {
		m_init_expr->Verify(ctx, VerifyContextParam());
	}
	VerifyContextResult vr_cond = m_cond_expr->Verify(ctx, VerifyContextParam().SetResultTid(TYPE_ID_BOOL));
	if (vr_cond.GetResultTypeId() != TYPE_ID_BOOL) {
		panicf("type of cond expr is not bool");
	}
	if (m_loop_expr != nullptr) {
		m_loop_expr->Verify(ctx, VerifyContextParam());
	}

	m_body->Verify(ctx, VerifyContextParam().SetReturnTid(vparam.GetReturnTid()));

	m_result_typeid = TYPE_ID_NONE;
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeFor::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	if (m_init_expr != nullptr) {
		m_init_expr->Execute(ctx);
	}
	while (true) {
		Variable* v_cond = m_cond_expr->Execute(ctx);
		if (v_cond->GetValueBool() == false) {
			break;
		}

		m_body->Execute(ctx);
		if (ctx.GetCurStack()->HasReturned()) {
			break;
		}

		if (m_loop_expr != nullptr) {
			m_loop_expr->Execute(ctx);
		}
	}
	ctx.GetCurStack()->LeaveBlock();
	return nullptr;
}
AstNodeFor* AstNodeFor::DeepCloneT() {
	return new AstNodeFor(m_init_expr == nullptr ? nullptr : m_init_expr->DeepClone(),
						  m_cond_expr == nullptr ? nullptr : m_cond_expr->DeepClone(),
						  m_loop_expr == nullptr ? nullptr : m_loop_expr->DeepClone(),
						  m_body->DeepClone());
}
