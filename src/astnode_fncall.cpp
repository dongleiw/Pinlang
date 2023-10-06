#include "astnode_fncall.h"
#include "astnode.h"
#include "astnode_access_attr.h"
#include "astnode_identifier.h"
#include "compile_context.h"
#include "define.h"
#include "function_obj.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/IR/Instructions.h>
#include <llvm-12/llvm/IR/Value.h>
#include <llvm-12/llvm/Support/raw_ostream.h>

AstNodeFnCall::AstNodeFnCall(AstNode* fn_expr, std::vector<AstNode*> args) {
	m_ast_type = AstType::FN_CALL;
	m_fn_expr  = fn_expr;
	m_args	   = args;

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
	VERIFY_BEGIN;
	log_info("verify fncall");

	TypeInfoFn* tifn = nullptr;

	VerifyContextResult vr;

	bool			   is_simple_fn		  = false;
	AstNodeIdentifier* astnode_identifier = dynamic_cast<AstNodeIdentifier*>(m_fn_expr);
	if (astnode_identifier != nullptr && astnode_identifier->IsSimpleFn(ctx)) {
		is_simple_fn = true;
	} else {
		AstNodeAccessAttr* astnode_access_attr = dynamic_cast<AstNodeAccessAttr*>(m_fn_expr);
		if (astnode_access_attr != nullptr && astnode_access_attr->IsSimpleFn(ctx)) {
			is_simple_fn = true;
		}
	}

	if (is_simple_fn) {
		// 这是一个简单函数(无重载,非泛型). 因此函数的类型是明确的. 从函数类型推导参数类型
		m_vr_fn	 = m_fn_expr->Verify(ctx, VerifyContextParam().SetExpectResultTgid(TYPE_GROUP_ID_FUNCTION));
		m_fn_tid = m_vr_fn.GetResultTypeId();
		tifn	 = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_fn_tid));
		if (tifn->GetParamNum() != m_args.size() + (m_vr_fn.HasSelfParam() ? 1 : 0)) {
			panicf("arg number not match");
		}
		for (size_t i = 0; i < m_args.size(); i++) {
			TypeId				arg_tid = tifn->GetParamType(i + (m_vr_fn.HasSelfParam() ? 1 : 0));
			VerifyContextResult vr_arg	= m_args.at(i)->Verify(ctx, VerifyContextParam().SetExpectResultTid(arg_tid));
			if (vr_arg.GetResultTypeId() != arg_tid) {
				panicf("arg[%lu] type not match: expect[%d:%s] give[%d:%s]", i, arg_tid, GET_TYPENAME_C(arg_tid), vr_arg.GetResultTypeId(), GET_TYPENAME_C(vr_arg.GetResultTypeId()));
			}
		}
		if (m_vr_fn.IsConstructor()) {
			m_result_typeid = m_vr_fn.GetObjTid();
		} else {
			m_result_typeid = tifn->GetReturnTypeId();
		}
	} else {
		// 复杂函数. 先检查参数, 然后根据参数类型选择合适的函数(重载)
		std::vector<TypeId> args_tid;
		for (auto iter : m_args) {
			VerifyContextResult vr_arg = iter->Verify(ctx, VerifyContextParam());
			args_tid.push_back(vr_arg.GetResultTypeId());
		}

		// 检查函数表达式
		m_vr_fn	 = m_fn_expr->Verify(ctx, VerifyContextParam().SetFnCallArgs(args_tid).SetExpectFnReturnTid(vparam.GetExpectResultTid()));
		m_fn_tid = m_vr_fn.GetResultTypeId();
		tifn	 = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_fn_tid));

		if (!tifn->VerifyArgsType(args_tid, m_vr_fn.HasSelfParam())) {
			panicf("argument type not match");
		}

		if (m_vr_fn.IsConstructor()) {
			vr.SetTmp(true);
			m_result_typeid = m_vr_fn.GetObjTid();
		} else {
			m_result_typeid = tifn->GetReturnTypeId();
		}
	}

	for (size_t i = 0; i < m_args.size(); i++) {
		TypeInfo* ti_arg = g_typemgr.GetTypeInfo(tifn->GetParamType(i + (m_vr_fn.HasSelfParam() ? 1 : 0)));
		if (ti_arg->IsArray()) {
			// 如果是数组, 会被修改为指向数组的指针. 因此这里需要设置compile_to_left_value
			m_args.at(i)->SetCompileToLeftValue();
		}
	}

	m_compile_to_left_value = vparam.ExpectLeftValue();

	vr.SetResultTypeId(m_result_typeid);

	return vr;
}
Variable* AstNodeFnCall::Execute(ExecuteContext& ctx) {
	panicf("no");
}
AstNodeFnCall* AstNodeFnCall::DeepCloneT() {
	std::vector<AstNode*> new_args;
	for (auto iter : m_args) {
		AstNode* newarg = iter->DeepClone();
		new_args.push_back(newarg);
	}

	AstNodeFnCall* newone = new AstNodeFnCall(m_fn_expr->DeepClone(), new_args);
	newone->Copy(*this);
	return newone;
}
CompileResult AstNodeFnCall::Compile(CompileContext& cctx) {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_fn_tid));

	CompileResult			  cr_fn = m_fn_expr->Compile(cctx);
	llvm::Function*			  fn	= cr_fn.GetResultFn();
	std::vector<llvm::Value*> args;

	llvm::Value* thisobj = nullptr;
	if (m_vr_fn.HasSelfParam()) {
		assert(fn->arg_size() == m_args.size() + 1); // 包括this
		if (m_vr_fn.IsConstructor()) {
			// 对于构造函数, 需要先创建一个obj
			TypeInfo* ti_obj = g_typemgr.GetTypeInfo(m_vr_fn.GetObjTid());
			thisobj			 = IRB.CreateAlloca(ti_obj->GetLLVMIRType(cctx), nullptr, "tmp_obj");
		} else {
			// 对于普通方法, obj已经创建了, 直接放入args
			thisobj = cr_fn.GetThisObj();
		}
		args.push_back(thisobj);
	} else {
		assert(fn->arg_size() == m_args.size());
	}

	for (size_t i = 0; i < m_args.size(); i++) {
		TypeId		  tid_arg = tifn->GetParamType(i);
		TypeInfo*	  ti_arg  = g_typemgr.GetTypeInfo(tid_arg);
		CompileResult cr_arg  = m_args.at(i)->Compile(cctx);
		if (m_vr_fn.IsMethod()) {
			assert(cr_arg.GetResult()->getType() == fn->getArg(i + 1)->getType());
		} else {
			assert(cr_arg.GetResult()->getType() == fn->getArg(i)->getType());
		}
		args.push_back(cr_arg.GetResult());
	}
	llvm::Value* call_ret = IRB.CreateCall(fn, args);
	if (m_vr_fn.IsConstructor()) {
		assert(thisobj != nullptr);
		if (m_compile_to_left_value) {
			return CompileResult().SetResult(thisobj);
		} else {
			TypeInfo*	ti_obj		= g_typemgr.GetTypeInfo(m_vr_fn.GetObjTid());
			llvm::Type* ir_type_obj = ti_obj->GetLLVMIRType(cctx);
			return CompileResult().SetResult(IRB.CreateLoad(ir_type_obj, thisobj));
		}
	} else if (fn->getReturnType()->isVoidTy()) {
		return CompileResult();
	} else {
		return CompileResult().SetResult(call_ret);
	}
}
