#include "astnode_return.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type_mgr.h"
#include "verify_context.h"

AstNodeReturn::AstNodeReturn(AstNode* returned_expr) {
	m_returned_expr		= returned_expr;
	m_returned_expr_tid = TYPE_ID_NONE;

	if (m_returned_expr != nullptr) {
		m_returned_expr->SetParent(this);
	}
}

VerifyContextResult AstNodeReturn::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	if (m_returned_expr != nullptr) {
		// 期望表达式的结果的类型 == return的类型
		VerifyContextParam vparam_return;
		vparam_return.SetReturnTid(vparam.GetReturnTid());
		VerifyContextResult vr_result = m_returned_expr->Verify(ctx, vparam_return);
		if (vparam.GetReturnTid() != TYPE_ID_INFER) {
			if (vr_result.GetResultTypeId() != vparam.GetReturnTid()) {
				panicf("type of return value is wrong. expect[%d:%s]. give[%d:%s]",
					   vparam.GetReturnTid(), GET_TYPENAME_C(vparam.GetReturnTid()),
					   vr_result.GetResultTypeId(), GET_TYPENAME_C(vr_result.GetResultTypeId()));
			}
		}
		m_returned_expr_tid	   = vr_result.GetResultTypeId();
		m_returned_expr_is_tmp = vr_result.IsTmp();
	}

	VerifyContextResult vr_result;
	return vr_result;
}
Variable* AstNodeReturn::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->SetReturnedValue(m_returned_expr->Execute(ctx));
	return nullptr;
}
AstNodeReturn* AstNodeReturn::DeepCloneT() {
	AstNodeReturn* newone = new AstNodeReturn();

	if (m_returned_expr != nullptr)
		newone->m_returned_expr = m_returned_expr->DeepClone();

	return newone;
}
CompileResult AstNodeReturn::Compile(VM& vm, FnInstructionMaker& maker) {
	if (m_returned_expr == nullptr) {
		maker.AddInstruction(new Instruction_ret<false>());
		maker.AddComment(InstructionComment(0, sprintf_to_stdstr("return;")));
	} else {
		maker.AddComment(InstructionComment(sprintf_to_stdstr("return xxx;")));
		TypeInfo* ti = g_typemgr.GetTypeInfo(m_returned_expr_tid);

		CompileResult cr_returned_expr = m_returned_expr->Compile(vm, maker);
		if (cr_returned_expr.IsFnId()) {
			panicf("not supported yet");
		} else {
			if (cr_returned_expr.IsValue()) {
				maker.AddInstruction(new Instruction_ret<true>(maker, cr_returned_expr.GetRegisterId(), ti->GetMemSize()));
			} else {
				maker.AddInstruction(new Instruction_ret<false>(maker, cr_returned_expr.GetRegisterId(), ti->GetMemSize()));
			}
			vm.ReleaseGeneralRegister(cr_returned_expr.GetRegisterId());
			if (!cr_returned_expr.GetStackVarName().empty()) {
				maker.VarEnd(cr_returned_expr.GetStackVarName());
			}
		}
	}
	return CompileResult();
}
