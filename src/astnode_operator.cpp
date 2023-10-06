#include "astnode_operator.h"
#include "astnode_compile_const.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
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

	if (vparam.ExpectLeftValue()) {
		panicf("`left_expr op right_expr` cannot be lvalue");
	}

	VerifyContextResult vr;

	TypeId		tid_left  = TYPE_ID_NONE;
	TypeId		tid_right = TYPE_ID_NONE;
	std::string fnid;

	if (!m_left_expr->IsLiteral() && m_right_expr->IsLiteral()) {
		// no_literal <operator> literal
		// 需要首先确定left的类型, 然后根据operator获取对应方法. 如果方法是泛型或者是函数重载, 则失败. 这种情况下需要显式声明常量值的具体类型
		// 如果方法是简单函数, 则根据参数来决定literal的类型

		VerifyContextResult vr_left = m_left_expr->Verify(ctx, VerifyContextParam());
		tid_left					= vr_left.GetResultTypeId();
		TypeInfo* ti_left			= g_typemgr.GetTypeInfo(tid_left);

		std::vector<std::string> method_indexs = ti_left->GetConstraintMethod(ctx, m_constraint_name, m_op);
		if (method_indexs.size() > 1) {
			panicf("multiple candidate");
		}
		if (method_indexs.empty()) {
			panicfi(m_si_op, "type[%d:%s] doesn't have method[%s:%s] ", tid_left, ti_left->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str());
		}
		fnid = method_indexs.at(0);

		const FnInfo* fn_info = ctx.GetFnTable().GetFnInfo(fnid);
		if (fn_info->GetParamNum_no_self() != 1) {
			panicf("type[%d:%s] doesn't have method[%s:%s] with 1 arg", tid_left, ti_left->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str());
		}

		VerifyContextResult vr_right = m_right_expr->Verify(ctx, VerifyContextParam().SetExpectResultTid(fn_info->GetParamType_no_self(0)));
		tid_right					 = vr_right.GetResultTypeId();

	} else if (m_left_expr->IsLiteral() && !m_right_expr->IsLiteral()) {
		// literal <operator> no_literal
		// 将literal转换为right类型
		VerifyContextResult vr_right = m_right_expr->Verify(ctx, VerifyContextParam());
		tid_right					 = vr_right.GetResultTypeId();

		VerifyContextResult vr_left = m_left_expr->Verify(ctx, VerifyContextParam().SetExpectResultTid(tid_right));
		tid_left					= vr_left.GetResultTypeId();

		TypeInfo*				 ti_left = g_typemgr.GetTypeInfo(tid_left);
		std::vector<TypeId>		 args_tid({tid_right});
		std::vector<std::string> method_indexs = ti_left->GetConstraintMethod(ctx, m_constraint_name, m_op, args_tid, TYPE_ID_INFER);
		if (method_indexs.size() > 1) {
			panicf("multiple candidate");
		}
		if (method_indexs.empty()) {
			panicfi(m_si_op, "type[%d:%s] doesn't have method[%s:%s] with args[%s]", tid_left,
					ti_left->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str(), g_typemgr.GetTypeName(args_tid).c_str());
		}
		fnid = method_indexs.at(0);
	} else {
		// no_literal <operator> no_literal
		// literal <operator> literal
		VerifyContextResult vr_left	 = m_left_expr->Verify(ctx, VerifyContextParam());
		VerifyContextResult vr_right = m_right_expr->Verify(ctx, VerifyContextParam());

		tid_left  = vr_left.GetResultTypeId();
		tid_right = vr_right.GetResultTypeId();

		TypeInfo* ti_left = g_typemgr.GetTypeInfo(tid_left);

		std::vector<TypeId> args_tid({tid_right});
		// 获取左侧类型实现的操作符对应的某个约束下的某个方法
		std::vector<std::string> method_indexs = ti_left->GetConstraintMethod(ctx, m_constraint_name, m_op, args_tid, TYPE_ID_INFER);
		if (method_indexs.size() > 1) {
			panicf("multiple candidate");
		}
		if (method_indexs.empty()) {
			panicfi(m_si_op, "type[%d:%s] doesn't have method[%s:%s] with args[%s]", tid_left,
					ti_left->GetName().c_str(), m_constraint_name.c_str(), m_op.c_str(), g_typemgr.GetTypeName(args_tid).c_str());
		}
		fnid = method_indexs.at(0);
	}

	// 方法有返回值
	TypeId return_tid = ctx.GetFnTable().GetFnReturnTypeId(fnid);
	if (return_tid == TYPE_ID_NONE) {
		panicf("type[%d:%s] method[%s:%s] return none", tid_left, GET_TYPENAME_C(tid_left), m_constraint_name.c_str(), m_op.c_str());
	}
	m_result_typeid = return_tid;
	vr.SetResultTypeId(m_result_typeid);

	log_debug("verify pass: type[%s] op[%s:%s] type[%s]", GET_TYPENAME_C(tid_left), m_constraint_name.c_str(), m_op.c_str(), GET_TYPENAME_C(tid_right));

	m_left_expr_tid	 = tid_left;
	m_right_expr_tid = tid_right;

	return vr;
}
AstNodeOperator* AstNodeOperator::DeepCloneT() {
	AstNodeOperator* newone = new AstNodeOperator(m_left_expr->DeepClone(), m_constraint_name, m_op, m_right_expr->DeepClone());
	newone->Copy(*this);
	return newone;
}
CompileResult AstNodeOperator::Compile(CompileContext& cctx) {
	if (m_compile_to_left_value) {
		panicf("`left_expr op right_expr` cannot be lvalue");
	}

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
		} else if (m_op == "mod") {
			if (is_unsigned_integer_type(m_left_expr_tid)) {
				result = IRB.CreateURem(left_value, right_value, "sub_result");
			} else {
				result = IRB.CreateSRem(left_value, right_value, "sub_result");
			}
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
	} else if (m_left_expr_tid == TYPE_ID_BOOL) {
		assert(m_left_expr_tid == m_right_expr_tid);
		if (m_op == "equal") {
			result = IRB.CreateICmpEQ(left_value, right_value, "eq_result");
		} else if (m_op == "notEqual") {
			result = IRB.CreateICmpNE(left_value, right_value, "ne_result");
		} else {
			panicf("not implemented yet");
		}
	} else {
		panicf("not implemented yet");
	}
	return CompileResult().SetResult(result);
}
