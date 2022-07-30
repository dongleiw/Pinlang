#include "astnode_operator.h"
#include "astnode_literal.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <vector>

AstNodeOperator::AstNodeOperator(AstNode* left_expr, std::string op, AstNode* right_expr) {
	m_left_expr	 = left_expr;
	m_right_expr = right_expr;
	m_op		 = op;
}

/*
 * TODO 将操作符的检查转换为方法调用的检查
 */
VerifyContextResult AstNodeOperator::Verify(VerifyContext& ctx) {
	log_debug("verify operator[%s]", m_op.c_str());
	VerifyContextResult vr;

	VerifyContextResult vr_left	 = m_left_expr->Verify(ctx);
	VerifyContextResult vr_right = m_right_expr->Verify(ctx);

	TypeId tid_left	 = vr_left.GetResultTypeId();
	TypeId tid_right = vr_right.GetResultTypeId();

	// 检查左表达式的add方法
	{
		TypeInfo* ti = g_typemgr.GetTypeInfo(tid_left);
		// 有对应方法
		Function* f = ti->GetMethodOrNilByName(m_op);
		if (f == nullptr) {
			panicf("type[%d:%s] doesn't have method %s", tid_left, ti->GetName().c_str(), m_op.c_str());
		}
		// 方法的参数类型匹配
		std::vector<TypeId> args_type;
		args_type.push_back(tid_right);
		if (!f->VerifyArgsType(args_type)) {
			panicf("type[%d:%s] method %s args not match", tid_left, ti->GetName().c_str(), m_op.c_str());
		}

		// 方法有返回值
		TypeId return_tid = f->GetReturnTypeId();
		if (return_tid == TYPE_ID_NONE) {
			panicf("type[%d:%s] method %s return none", tid_left, ti->GetName().c_str(), m_op.c_str());
		}
		m_result_typeid = return_tid;
		vr.SetResultTypeId(m_result_typeid);
	}
	return vr;
}
Variable* AstNodeOperator::Execute(ExecuteContext& ctx) {
	Variable*			   left_v  = m_left_expr->Execute(ctx);
	Variable*			   right_v = m_right_expr->Execute(ctx);
	std::vector<Variable*> args;
	args.push_back(right_v);
	Variable* result = left_v->CallMethod(ctx, m_op, args);
	log_debug("%d.%s(%d)=>%s", left_v->GetValueInt(), m_op.c_str(), right_v->GetValueInt(), result->ToString().c_str());
	return result;
}
