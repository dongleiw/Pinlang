#include "astnode_operator.h"
#include "astnode_literal.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <memory>
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
		// 生成uniq_method_name
		std::vector<TypeId> args_tid;
		args_tid.push_back(tid_right);
		std::string uniq_method_name = m_op +"_"+ TypeInfoFn::GetUniqFnNameSuffix(args_tid);

		TypeInfo* ti = g_typemgr.GetTypeInfo(tid_left);
		// 有对应方法
		Function* f = ti->GetMethodOrNilByName(uniq_method_name);
		if (f == nullptr) {
			panicf("type[%d:%s] doesn't have method[%s:%s]", tid_left, ti->GetName().c_str(), m_op.c_str(), uniq_method_name.c_str());
		}
		if (!f->VerifyArgsType(args_tid)) {
			panicf("type[%d:%s] method[%s:%s] args not match", tid_left, ti->GetName().c_str(), m_op.c_str(), uniq_method_name.c_str());
		}

		// 方法有返回值
		TypeId return_tid = f->GetReturnTypeId();
		if (return_tid == TYPE_ID_NONE) {
			panicf("type[%d:%s] method[%s:%s] return none", tid_left, ti->GetName().c_str(), m_op.c_str(), uniq_method_name.c_str());
		}
		m_result_typeid = return_tid;
		vr.SetResultTypeId(m_result_typeid);
		m_uniq_method_name = uniq_method_name;

		log_debug("verify pass: type[%s] op[%s:%s] type[%s]", GET_TYPENAME_C(tid_left),m_op.c_str(), m_uniq_method_name.c_str(), GET_TYPENAME_C(tid_right));
	}
	return vr;
}
Variable* AstNodeOperator::Execute(ExecuteContext& ctx) {
	Variable*			   left_v  = m_left_expr->Execute(ctx);
	Variable*			   right_v = m_right_expr->Execute(ctx);
	std::vector<Variable*> args;
	args.push_back(right_v);
	Variable* result = left_v->CallMethod(ctx, m_uniq_method_name, args);
	log_debug("%d.%s(%d)=>%s", left_v->GetValueInt(), m_op.c_str(), right_v->GetValueInt(), result->ToString().c_str());
	return result;
}
