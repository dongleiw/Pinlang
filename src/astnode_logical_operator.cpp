#include "astnode_logical_operator.h"
#include "astnode_compile_const.h"
#include "astnode_literal.h"
#include "define.h"
#include "execute_context.h"
#include "function.h"
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
	log_debug("verify operator[%s]", m_op.c_str());
	VerifyContextResult vr;

	VerifyContextResult vr_left	 = m_left_expr->Verify(ctx, VerifyContextParam().SetResultTid(TYPE_ID_BOOL));
	VerifyContextResult vr_right = m_right_expr->Verify(ctx, VerifyContextParam().SetResultTid(TYPE_ID_BOOL));

	if (vr_left.GetResultTypeId() != TYPE_ID_BOOL) {
		panicf("should be bool");
	}
	if (vr_right.GetResultTypeId() != TYPE_ID_BOOL) {
		panicf("should be bool");
	}

	// 处理编译期常量表达式
	if (vr_left.IsConst()) {
		m_left_expr = new AstNodeCompileConst(vr_left.GetConstResult());
	}
	if (vr_right.IsConst()) {
		m_right_expr = new AstNodeCompileConst(vr_right.GetConstResult());
	}
	if (m_op == "and") {
		if (vr_left.IsConst()) {
			if (vr_left.GetConstResult()->GetValueBool() == false) {
				vr.SetConstResult(vr_left.GetConstResult());
			} else if (vr_right.IsConst()) {
				vr.SetConstResult(vr_right.GetConstResult());
			}
		}
	} else if (m_op == "or") {
		if (vr_left.IsConst()) {
			if (vr_left.GetConstResult()->GetValueBool() == true) {
				vr.SetConstResult(vr_left.GetConstResult());
			} else if (vr_right.IsConst()) {
				vr.SetConstResult(vr_right.GetConstResult());
			}
		}
	} else {
		panicf("unknown op");
	}

	vr.SetResultTypeId(TYPE_ID_BOOL);
	return vr;
}
Variable* AstNodeLogicalOperator::Execute(ExecuteContext& ctx) {
	if (m_op == "and") {
		Variable* left_v = m_left_expr->Execute(ctx);
		if (left_v->GetValueBool() == false) {
			return left_v;
		} else {
			return m_right_expr->Execute(ctx);
		}
	} else if (m_op == "or") {
		Variable* left_v = m_left_expr->Execute(ctx);
		if (left_v->GetValueBool() == true) {
			return left_v;
		} else {
			return m_right_expr->Execute(ctx);
		}
	} else {
		panicf("unknown op");
	}
}
AstNodeLogicalOperator* AstNodeLogicalOperator::DeepCloneT() {
	AstNodeLogicalOperator* newone = new AstNodeLogicalOperator(m_left_expr->DeepClone(), m_op, m_right_expr->DeepClone());
	return newone;
}
