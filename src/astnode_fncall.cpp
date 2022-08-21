#include "astnode_fncall.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"

AstNodeFnCall::AstNodeFnCall(AstNode* fn_expr, std::vector<AstNode*> args) {
	m_fn_expr = fn_expr;
	m_args	  = args;
}

VerifyContextResult AstNodeFnCall::Verify(VerifyContext& ctx) {
	log_info("verify fncall");

	// 先检查参数, 然后根据参数类型选择合适的函数(重载)
	std::vector<TypeId> args_tid;
	for (auto iter : m_args) {
		VerifyContextResult vr_arg = iter->Verify(ctx);
		args_tid.push_back(vr_arg.GetResultTypeId());
	}

	// 检查函数表达式
	ctx.GetParam().Clear();
	ctx.GetParam().SetFnCallArgs(args_tid);
	VerifyContextResult vr_fn_expr = m_fn_expr->Verify(ctx);
	TypeId				fn_tid	   = vr_fn_expr.GetResultTypeId();
	TypeInfoFn*			tifn	   = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));

	if (!tifn->VerifyArgsType(args_tid)) {
		panicf("argument type not match");
	}

	m_result_typeid = tifn->GetReturnTypeId();
	VerifyContextResult vr(m_result_typeid);

	return vr;
}
Variable* AstNodeFnCall::Execute(ExecuteContext& ctx) {
	log_debug("fncall");
	Variable*			   fn_var = m_fn_expr->Execute(ctx);
	Function*			   fn	  = fn_var->GetValueFunction();
	std::vector<Variable*> args;
	for (auto iter : m_args) {
		args.push_back(iter->Execute(ctx));
	}
	return fn->Call(ctx, args);
}
