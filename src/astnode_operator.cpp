#include "astnode_operator.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

AstNodeOperator::AstNodeOperator(AstNode* left_expr, std::string op, AstNode* right_expr) {
	m_left_expr = left_expr;
	m_right_expr = right_expr;
	m_op = op;
}

/*
 * TODO 将操作符的检查转换为方法调用的检查
 */
VerifyContextResult AstNodeOperator::Verify(VerifyContext& ctx) {
	log_debug("verify operator[%s]", m_op.c_str());
	VerifyContextResult vr;

	VerifyContextResult vr_left = m_left_expr->Verify(ctx);
	VerifyContextResult vr_right = m_right_expr->Verify(ctx);

	TypeId tid_left = vr_left.GetResultTypeId();
	TypeId tid_right = vr_right.GetResultTypeId();

	if (tid_left != tid_right) {
		panicf("not support yet: left[%s] right[%s]", get_typedesc(tid_left), get_typedesc(tid_right));
	}

	if (tid_left == TYPE_ID_INT) {
		if (m_op == "+") {
		} else if (m_op == "-") {
		} else if (m_op == "*") {
		} else if (m_op == "/") {
		} else if (m_op == "%") {
		} else {
			panicf("int not support op[%s]", m_op.c_str());
		}
		m_result_typeid = TYPE_ID_INT;
		vr.SetResultTypeId(TYPE_ID_INT);
	} else {
		panicf("unknown type of left expr[%d]", tid_left);
	}
	return vr;
}
Variable* AstNodeOperator::Execute(ExecuteContext& ctx) {
	if (m_result_typeid == TYPE_ID_INT) {
		Variable* left_v = m_left_expr->Execute(ctx);
		Variable* right_v = m_right_expr->Execute(ctx);
		int		  result;
		if (m_op == "+") {
			result = left_v->GetValueInt() + right_v->GetValueInt();
		} else if (m_op == "-") {
			result = left_v->GetValueInt() - right_v->GetValueInt();
		} else if (m_op == "*") {
			result = left_v->GetValueInt() * right_v->GetValueInt();
		} else if (m_op == "/") {
			result = left_v->GetValueInt() / right_v->GetValueInt();
		} else if (m_op == "%") {
			result = left_v->GetValueInt() % right_v->GetValueInt();
		} else {
			panicf("int not support op[%s]", m_op.c_str());
		}
		log_debug("%d%s%d=%d", left_v->GetValueInt(), m_op.c_str(), right_v->GetValueInt(), result);
		return new Variable(result);
	} else {
		panicf("unknown type of left expr[%d]", m_result_typeid);
	}
	return NULL;
}
