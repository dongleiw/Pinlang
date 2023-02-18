#include "astnode_operator.h"
#include "astnode_compile_const.h"
#include "astnode_literal.h"
#include "define.h"
#include "execute_context.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "verify_context.h"
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

	VerifyContextResult vr_left	 = m_left_expr->Verify(ctx, VerifyContextParam());
	VerifyContextResult vr_right = m_right_expr->Verify(ctx, VerifyContextParam());

	TypeId tid_left	 = vr_left.GetResultTypeId();
	TypeId tid_right = vr_right.GetResultTypeId();

	TypeInfo* ti = g_typemgr.GetTypeInfo(tid_left);

	std::vector<TypeId> args_tid({tid_right});
	// 获取左侧类型实现的操作符对应的某个约束下的某个方法
	std::vector<MethodIndex> method_indexs = ti->GetConstraintMethod(ctx, m_constraint_name, m_op, args_tid);
	if (method_indexs.size() > 1) {
		panicf("multiple candidate");
	}
	if (method_indexs.empty()) {
		panicfi(m_si_op, "type[%d:%s] doesn't have method[%s:%s] with args[%s]", tid_left,
				ti->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str(), g_typemgr.GetTypeName(args_tid).c_str());
	}
	MethodIndex method_idx = method_indexs.at(0);

	// 检查左表达式的operator对应方法
	if (!method_idx.IsValid()) {
		panicfi(m_si_op, "type[%d:%s] doesn't have method[%s:%s] with args[%s]", tid_left,
				ti->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str(), g_typemgr.GetTypeName(args_tid).c_str());
	}
	m_fn_addr = ti->GetMethodByIdx(method_idx);

	// 方法有返回值
	TypeId return_tid = ctx.GetFnTable().GetFnReturnTypeId(m_fn_addr);
	if (return_tid == TYPE_ID_NONE) {
		panicf("type[%d:%s] method[%s:%s] return none", tid_left, ti->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str());
	}
	m_result_typeid = return_tid;
	vr.SetResultTypeId(m_result_typeid);

	log_debug("verify pass: type[%s] op[%s:%s] type[%s]", GET_TYPENAME_C(tid_left), m_constraint_name.c_str(), m_op.c_str(), GET_TYPENAME_C(tid_right));

	// 处理编译期常量表达式
	// 必须是基本类型. 否则是operator overloading. 目前没法确认是否是纯方法. 如果不是纯方法, `left op right`也不是编译期常量表达式
	//if (vr_left.IsConst()) {
	//	m_left_expr = new AstNodeCompileConst(vr_left.GetConstResult());
	//}
	//if (vr_right.IsConst()) {
	//	m_right_expr = new AstNodeCompileConst(vr_right.GetConstResult());
	//}
	//if (ti->IsPrimaryType() && vr_left.IsConst() && vr_right.IsConst()) {
	//	// 这里临时搞了一个ExecuteContext用来调用方法. TODO 太丑陋了
	//	ExecuteContext exe_ctx;
	//	exe_ctx.SetFnTable(ctx.GetFnTable());
	//	Variable*			   v_left = vr_left.GetConstResult();
	//	std::vector<Variable*> args{vr_right.GetConstResult()};
	//	Variable*			   v_result = exe_ctx.GetFnTable().CallFn(m_fn_addr, exe_ctx, v_left, args);
	//	vr.SetConstResult(v_result);
	//}

	m_left_expr_tid	 = vr_left.GetResultTypeId();
	m_right_expr_tid = vr_right.GetResultTypeId();

	m_left_expr_is_tmp	= vr_left.IsTmp();
	m_right_expr_is_tmp = vr_right.IsTmp();

	return vr;
}
Variable* AstNodeOperator::Execute(ExecuteContext& ctx) {
	Variable*			   left_v  = m_left_expr->Execute(ctx);
	Variable*			   right_v = m_right_expr->Execute(ctx);
	std::vector<Variable*> args;
	args.push_back(right_v);
	Variable* result = ctx.GetFnTable().CallFn(m_fn_addr, ctx, left_v, args);
	log_debug("%s.%s(%s)=>%s", left_v->ToString().c_str(), m_op.c_str(), right_v->ToString().c_str(), result->ToString().c_str());
	return result;
}
AstNodeOperator* AstNodeOperator::DeepCloneT() {
	AstNodeOperator* newone = new AstNodeOperator(m_left_expr->DeepClone(), m_constraint_name, m_op, m_right_expr->DeepClone());
	return newone;
}
CompileResult AstNodeOperator::Compile(VM& vm, FnInstructionMaker& maker) {
	if ((m_left_expr_tid == TYPE_ID_INT32 && m_right_expr_tid == TYPE_ID_INT32) || (m_left_expr_tid == TYPE_ID_INT64 && m_right_expr_tid == TYPE_ID_INT64) ||
		(m_left_expr_tid == TYPE_ID_UINT32 && m_right_expr_tid == TYPE_ID_UINT32) ||
		(m_left_expr_tid == TYPE_ID_UINT64 && m_right_expr_tid == TYPE_ID_UINT64)) {
		//TypeInfo* ti = g_typemgr.GetTypeInfo(TYPE_ID_INT32);

		maker.AddComment(InstructionComment(sprintf_to_stdstr("xx + xx")));
		RegisterId register_result = vm.AllocGeneralRegister();

		CompileResult cr_left  = m_left_expr->Compile(vm, maker);
		CompileResult cr_right = m_right_expr->Compile(vm, maker);

		if (cr_left.IsFnId() || cr_right.IsFnId()) {
			panicf("not implemented yet");
		} else {

#define _generate_add_instruction(tid, result_is_value, left_is_value, right_is_value)                                                                                                  \
	if (tid == TYPE_ID_INT32 || tid == TYPE_ID_UINT32) {                                                                                                                                \
		maker.AddInstruction(new Instruction_add<uint32_t, result_is_value, left_is_value, right_is_value>(maker, register_result, cr_left.GetRegisterId(), cr_right.GetRegisterId())); \
	} else if (tid == TYPE_ID_INT64 || tid == TYPE_ID_UINT64) {                                                                                                                         \
		maker.AddInstruction(new Instruction_add<uint64_t, result_is_value, left_is_value, right_is_value>(maker, register_result, cr_left.GetRegisterId(), cr_right.GetRegisterId())); \
	}

			if (cr_left.IsValue()) {
				if (cr_right.IsValue()) {
					_generate_add_instruction(m_left_expr_tid, true, true, true);
				} else {
					_generate_add_instruction(m_left_expr_tid, true, true, false);
				}
			} else {
				if (cr_right.IsValue()) {
					_generate_add_instruction(m_left_expr_tid, true, false, true);
				} else {
					_generate_add_instruction(m_left_expr_tid, true, false, false);
				}
			}
		}

		vm.ReleaseGeneralRegister(cr_left.GetRegisterId());
		vm.ReleaseGeneralRegister(cr_right.GetRegisterId());

		if (!cr_left.GetStackVarName().empty()) {
			maker.VarEnd(cr_left.GetStackVarName());
		}
		if (!cr_right.GetStackVarName().empty()) {
			maker.VarEnd(cr_right.GetStackVarName());
		}
		return CompileResult(register_result, true, "");
	} else {
		panicf("not implemented yet");
	}
}
