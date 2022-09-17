#include "astnode_operator.h"
#include "astnode_literal.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <memory>
#include <vector>

AstNodeOperator::AstNodeOperator(AstNode* left_expr, std::string constraint_name, std::string op, AstNode* right_expr) {
	m_left_expr		  = left_expr;
	m_constraint_name = constraint_name;
	m_op			  = op;
	m_right_expr	  = right_expr;

	m_left_expr->SetParent(this);
	m_right_expr->SetParent(this);
}

/*
 * 调用函数
 */
VerifyContextResult AstNodeOperator::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify operator[%s]", m_op.c_str());
	VerifyContextResult vr;

	VerifyContextResult vr_left	 = m_left_expr->Verify(ctx,VerifyContextParam());
	VerifyContextResult vr_right = m_right_expr->Verify(ctx,VerifyContextParam());

	TypeId tid_left	 = vr_left.GetResultTypeId();
	TypeId tid_right = vr_right.GetResultTypeId();

	// 检查左表达式的operator对应方法
	{
		std::vector<TypeId> args_tid;
		args_tid.push_back(tid_right);

		TypeInfo* ti = g_typemgr.GetTypeInfo(tid_left);

		MethodIndex method_idx;
		if (m_constraint_name.empty()) {
			method_idx = ti->GetMethodIdx(m_op, args_tid);
		} else {
			TypeId constraint_tid = ctx.GetCurStack()->GetVariableType(m_constraint_name);
			method_idx			  = ti->GetMethodIdx(constraint_tid, m_op, args_tid);
		}
		if (!method_idx.IsValid()) {
			panicf("type[%d:%s] doesn't have method[%s:%s] with args[%s]", tid_left, ti->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str(), g_typemgr.GetTypeName(args_tid).c_str());
		}
		Function* f = ti->GetMethodByIdx(method_idx);

		// 方法有返回值
		TypeId return_tid = f->GetReturnTypeId();
		if (return_tid == TYPE_ID_NONE) {
			panicf("type[%d:%s] method[%s:%s] return none", tid_left, ti->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str());
		}
		m_result_typeid = return_tid;
		vr.SetResultTypeId(m_result_typeid);

		m_method_idx = method_idx;

		log_debug("verify pass: type[%s] op[%s:%s] type[%s]", GET_TYPENAME_C(tid_left), m_constraint_name.c_str(), m_op.c_str(), GET_TYPENAME_C(tid_right));
	}
	return vr;
}
Variable* AstNodeOperator::Execute(ExecuteContext& ctx) {
	Variable*			   left_v  = m_left_expr->Execute(ctx);
	Variable*			   right_v = m_right_expr->Execute(ctx);
	std::vector<Variable*> args;
	args.push_back(right_v);
	Variable* result = left_v->CallMethod(ctx, m_method_idx, args);
	log_debug("%s.%s(%s)=>%s", left_v->ToString().c_str(), m_op.c_str(), right_v->ToString().c_str(), result->ToString().c_str());
	return result;
}
AstNodeOperator* AstNodeOperator::DeepCloneT() {
	AstNodeOperator* newone = new AstNodeOperator(m_left_expr->DeepClone(), m_constraint_name, m_op, m_right_expr->DeepClone());
	return newone;
}
