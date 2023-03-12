#include "astnode_operator.h"
#include "astnode_compile_const.h"
#include "astnode_literal.h"
#include "compile_context.h"
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
#include <llvm-12/llvm/IR/Type.h>
#include <llvm-12/llvm/IR/Value.h>
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
CompileResult AstNodeOperator::Compile(CompileContext& cctx) {
	CompileResult cr_left_value	 = m_left_expr->Compile(cctx);
	CompileResult cr_right_value = m_right_expr->Compile(cctx);

	llvm::Value* left_value	 = cr_left_value.GetResult();
	llvm::Value* right_value = cr_right_value.GetResult();

	TypeInfo* ti_left  = g_typemgr.GetTypeInfo(m_left_expr_tid);
	TypeInfo* ti_right = g_typemgr.GetTypeInfo(m_right_expr_tid);

	llvm::Type* ir_type_left  = ti_left->GetLLVMIRType(cctx);
	llvm::Type* ir_type_right = ti_right->GetLLVMIRType(cctx);

	assert(left_value->getType() == ir_type_left && right_value->getType() == ir_type_right);

	llvm::Value* result = nullptr;
	if (is_integer_type(m_left_expr_tid)) {
		assert(m_left_expr_tid == m_right_expr_tid);
		if (m_op == "add") {
			result = IRB.CreateAdd(left_value, right_value, "add_result");
		} else if (m_op == "sub") {
			result = IRB.CreateSub(left_value, right_value, "sub_result");
		} else if (m_op == "mul") {
			result = IRB.CreateMul(left_value, right_value, "mul_result");
		} else if (m_op == "div") {
			if (is_unsigned_integer_type(m_left_expr_tid)) {
				result = IRB.CreateUDiv(left_value, right_value, "div_result");
			} else if (is_signed_integer_type(m_left_expr_tid)) {
				result = IRB.CreateSDiv(left_value, right_value, "div_result");
			} else {
				panicf("not implemented yet");
			}
		} else if (m_op == "equal") {
			result = IRB.CreateICmpEQ(left_value, right_value, "eq_result");
		} else if (m_op == "notEqual") {
			result = IRB.CreateICmpNE(left_value, right_value, "ne_result");
		} else if (m_op == "lessThan") {
			if (is_unsigned_integer_type(m_left_expr_tid)) {
				result = IRB.CreateICmpULT(left_value, right_value, "lt_result");
			} else if (is_signed_integer_type(m_left_expr_tid)) {
				result = IRB.CreateICmpSLT(left_value, right_value, "lt_result");
			} else {
				panicf("not implemented yet");
			}
		} else if (m_op == "lessEqual") {
			if (is_unsigned_integer_type(m_left_expr_tid)) {
				result = IRB.CreateICmpULE(left_value, right_value, "le_result");
			} else if (is_signed_integer_type(m_left_expr_tid)) {
				result = IRB.CreateICmpSLE(left_value, right_value, "le_result");
			} else {
				panicf("not implemented yet");
			}
		} else if (m_op == "greaterThan") {
			if (is_unsigned_integer_type(m_left_expr_tid)) {
				result = IRB.CreateICmpUGT(left_value, right_value, "gt_result");
			} else if (is_signed_integer_type(m_left_expr_tid)) {
				result = IRB.CreateICmpSGT(left_value, right_value, "gt_result");
			} else {
				panicf("not implemented yet");
			}
		} else if (m_op == "greaterEqual") {
			if (is_unsigned_integer_type(m_left_expr_tid)) {
				result = IRB.CreateICmpUGE(left_value, right_value, "ge_result");
			} else if (is_signed_integer_type(m_left_expr_tid)) {
				result = IRB.CreateICmpSGE(left_value, right_value, "ge_result");
			} else {
				panicf("not implemented yet");
			}
		} else {
			panicf("not implemented yet");
		}
	} else {
		panicf("not implemented yet");
	}
	return CompileResult().SetResult(result);
}
