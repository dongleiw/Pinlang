#include "astnode_fncall.h"
#include "define.h"
#include "function_obj.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "utils.h"
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/IR/Instructions.h>
#include <llvm-12/llvm/IR/Value.h>
#include <llvm-12/llvm/Support/raw_ostream.h>

AstNodeFnCall::AstNodeFnCall(AstNode* fn_expr, std::vector<AstNode*> args) {
	m_fn_expr = fn_expr;
	m_args	  = args;

	m_fn_expr->SetParent(this);
	for (auto iter : m_args) {
		iter->SetParent(this);
	}
}

/*
 * 函数调用检查
 * 可能需要根据调用的参数从函数重载列表中选择一个, 也可能根据函数的参数类型来推导出参数的类型
 */
VerifyContextResult AstNodeFnCall::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_info("verify fncall");

	// 先检查参数, 然后根据参数类型选择合适的函数(重载)
	std::vector<TypeId> args_tid;
	for (auto iter : m_args) {
		VerifyContextResult vr_arg = iter->Verify(ctx, VerifyContextParam());
		args_tid.push_back(vr_arg.GetResultTypeId());
		m_arg_is_tmp.push_back(vr_arg.IsTmp());
	}

	// 检查函数表达式
	VerifyContextResult vr_fn_expr = m_fn_expr->Verify(ctx, VerifyContextParam().SetFnCallArgs(args_tid).SetResultTid(vparam.GetResultTid()));
	m_fn_tid					   = vr_fn_expr.GetResultTypeId();
	TypeInfoFn* tifn			   = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_fn_tid));

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
	FunctionObj*		   fnobj  = fn_var->GetValueFunctionObj();
	std::vector<Variable*> args;
	for (auto iter : m_args) {
		args.push_back(iter->Execute(ctx));
	}
	return fnobj->Call(ctx, args);
}
AstNodeFnCall* AstNodeFnCall::DeepCloneT() {
	AstNodeFnCall* newone = new AstNodeFnCall();

	newone->m_fn_expr = m_fn_expr->DeepClone();
	for (auto iter : m_args) {
		newone->m_args.push_back(iter->DeepClone());
	}

	return newone;
}
llvm::Value* AstNodeFnCall::Compile(CompileContext& cctx) {
	llvm::Function* fn = (llvm::Function*)m_fn_expr->Compile(cctx);
	assert(fn->arg_size() == m_args.size());
	std::vector<llvm::Value*> args;
	for (size_t i = 0; i < m_args.size(); i++) {
		llvm::Value* arg_value = m_args.at(i)->Compile(cctx);
		if (arg_value->getType() == fn->getArg(i)->getType()) {
			args.push_back(arg_value);
		} else if (arg_value->getType() == fn->getArg(i)->getType()->getPointerTo()) {
			args.push_back(IRB.CreateLoad(fn->getArg(i)->getType(), arg_value, sprintf_to_stdstr("load_arg_%lu", i)));
		} else {
			arg_value->getType()->print(llvm::outs(), true);
			printf("\n");
			fn->getArg(i)->getType()->print(llvm::outs(), true);
			printf("\n");
			panicf("bug");
		}
	}
	if (fn->getReturnType()->isVoidTy()) {
		IRB.CreateCall((llvm::Function*)fn, args); // return void 的情况下名字必须为空
		return nullptr;
	} else {
		return IRB.CreateCall((llvm::Function*)fn, args, "fn_call_ret");
	}
}
