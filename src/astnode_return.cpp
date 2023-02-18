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
void AstNodeReturn::Compile(VM& vm, FnInstructionMaker& maker, MemAddr& target_addr) {
	// return是statement, 忽略returned_var

	if (m_returned_expr == nullptr) {
		maker.AddInstruction(new Instruction_ret());
	} else {
		TypeInfo* ti = g_typemgr.GetTypeInfo(m_returned_expr_tid);

		if (m_returned_expr_is_tmp) {
			Var tmpv = maker.TmpVarBegin(ti->GetMemSize());
			m_returned_expr->Compile(vm, maker, tmpv.mem_addr);
			maker.AddInstruction(new Instruction_ret(ti->GetMemSize(), tmpv.mem_addr));
		} else {
			MemAddr mem_addr;
			m_returned_expr->Compile(vm, maker, mem_addr);
			maker.AddInstruction(new Instruction_ret(ti->GetMemSize(), mem_addr));
		}
	}
}
