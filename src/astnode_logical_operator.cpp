#include "astnode_logical_operator.h"
#include "astnode.h"
#include "astnode_compile_const.h"
#include "define.h"
#include "execute_context.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <memory>
#include <vector>

AstNodeLogicalOperator::AstNodeLogicalOperator(AstNode* left_expr, std::string op, AstNode* right_expr) {
	m_left_expr	 = left_expr;
	m_op		 = op;
	m_right_expr = right_expr;

	m_left_expr->SetParent(this);
	m_right_expr->SetParent(this);
}

/*
 * 调用函数
 */
VerifyContextResult AstNodeLogicalOperator::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	log_debug("verify operator[%s]", m_op.c_str());
	VerifyContextResult vr;

	VerifyContextResult vr_left	 = m_left_expr->Verify(ctx, VerifyContextParam().SetExpectResultTid(TYPE_ID_BOOL));
	VerifyContextResult vr_right = m_right_expr->Verify(ctx, VerifyContextParam().SetExpectResultTid(TYPE_ID_BOOL));

	if (vr_left.GetResultTypeId() != TYPE_ID_BOOL) {
		panicf("should be bool");
	}
	if (vr_right.GetResultTypeId() != TYPE_ID_BOOL) {
		panicf("should be bool");
	}

	vr.SetResultTypeId(TYPE_ID_BOOL);
	return vr;
}
Variable* AstNodeLogicalOperator::Execute(ExecuteContext& ctx) {
	panicf("");
}
AstNodeLogicalOperator* AstNodeLogicalOperator::DeepCloneT() {
	AstNodeLogicalOperator* newone = new AstNodeLogicalOperator(m_left_expr->DeepClone(), m_op, m_right_expr->DeepClone());
	return newone;
}
