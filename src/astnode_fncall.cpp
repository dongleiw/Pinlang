#include "astnode_fncall.h"
#include "define.h"
#include "function_obj.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "utils.h"

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
CompileResult AstNodeFnCall::Compile(VM& vm, FnInstructionMaker& maker) {
	CompileResult cr_fn = m_fn_expr->Compile(vm, maker);
	if (!cr_fn.IsFnId()) {
		panicf("not supported yet");
	}
	std::string fn_id = cr_fn.GetFnId();
	maker.AddComment(InstructionComment(sprintf_to_stdstr("call %s", fn_id.c_str())));

	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_fn_tid));

	Var ret_var;
	if (m_result_typeid != TYPE_ID_NONE) {
		TypeInfo* ret_ti = g_typemgr.GetTypeInfo(m_result_typeid);
		ret_var			 = maker.TmpVarBegin("fn_return", ret_ti->GetMemSize());
	}

	// 压入参数
	std::vector<Var> push_arg_list;
	for (size_t i = 0; i < m_args.size(); i++) {
		TypeId	  arg_tid = tifn->GetParamType(i);
		TypeInfo* arg_ti  = g_typemgr.GetTypeInfo(arg_tid);

		// 申请一块内存, 作为callee的参数
		Var var_arg = maker.TmpVarBegin("fn_arg", arg_ti->GetMemSize());
		maker.AddComment(InstructionComment(sprintf_to_stdstr("push arg %s", to_str(i).c_str())));

		RegisterId register_arg_addr = vm.AllocGeneralRegister();
		maker.AddInstruction(new Instruction_add_const<uint64_t, true, true>(maker, register_arg_addr, REGISTER_ID_STACK_FRAME, var_arg.mem_addr.relative_addr,
																			 sprintf_to_stdstr("get addr of arg[%d]", i)));

		// 编译第i个参数, 得到一个内存地址, 该内存地址对应内存块保存了该参数的值
		CompileResult cr_arg = m_args.at(i)->Compile(vm, maker);
		if (cr_arg.IsFnId()) {
			panicf("not implemented yet");
		} else {
			if (cr_arg.IsValue()) {
				maker.AddInstruction(new Instruction_store_register(maker, register_arg_addr, cr_arg.GetRegisterId(), arg_ti->GetMemSize(),
																	sprintf_to_stdstr("push value of arg-expr to stack: arg-idx[%d]", i)));
			} else {
				maker.AddInstruction(new Instruction_memcpy(maker, register_arg_addr, cr_arg.GetRegisterId(), arg_ti->GetMemSize(),
															sprintf_to_stdstr("push value of arg-expr to stack: arg-idx[%d]", i)));
			}
			vm.ReleaseGeneralRegister(cr_arg.GetRegisterId());
			if (!cr_arg.GetStackVarName().empty()) {
				maker.VarEnd(cr_arg.GetStackVarName());
			}
		}

		vm.ReleaseGeneralRegister(register_arg_addr);

		push_arg_list.push_back(var_arg);
	}

	if (m_result_typeid == TYPE_ID_NONE) {
		// 函数没有返回值. 传递一个无效的return-var-memaddr
		maker.AddInstruction(new Instruction_call(maker, fn_id, 0));
	} else {
		// 函数有返回值. 函数调用的结果是临时变量, 将函数执行的结果保存到外层提供的target_addr
		maker.AddInstruction(new Instruction_call(maker, fn_id, ret_var.mem_addr.relative_addr));
	}

	// 参数弹出栈
	for (size_t i = push_arg_list.size(); i > 0; i--) {
		maker.VarEnd(push_arg_list.at(i - 1).var_name);
	}

	RegisterId register_returned_var_addr = vm.AllocGeneralRegister();
	maker.AddInstruction(new Instruction_add_const<uint64_t, true, true>(maker, register_returned_var_addr, REGISTER_ID_STACK_FRAME, ret_var.mem_addr.relative_addr));
	if (m_result_typeid != TYPE_ID_NONE) {
		return CompileResult(register_returned_var_addr, false, ret_var.var_name);
	} else {
		return CompileResult();
	}
}
