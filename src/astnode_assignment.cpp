#include "astnode_assignment.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

VerifyContextResult AstNodeAssignment::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify assigment");

	VerifyContextResult vr_left = m_left->Verify(ctx, VerifyContextParam());
	if (vr_left.GetResultTypeId() == TYPE_ID_NONE) {
		panicf("type of left expr in assignment is none");
	}
	if(vr_left.IsTmp()){
		panicf("result of left expr in assignment is tmp");
	}

	VerifyContextResult vr_right = m_right->Verify(ctx, VerifyContextParam().SetResultTid(vr_left.GetResultTypeId()));
	if (vr_right.GetResultTypeId() != vr_left.GetResultTypeId()) {
		panicf("type not match in assignment: left[%d:%s] right[%d:%s]", vr_left.GetResultTypeId(), GET_TYPENAME_C(vr_left.GetResultTypeId()),
			   vr_right.GetResultTypeId(), GET_TYPENAME_C(vr_right.GetResultTypeId()));
	}

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeAssignment::Execute(ExecuteContext& ctx) {
	Variable* v_right= m_right->Execute(ctx);
	ctx.SetAssignValue(v_right);
	m_left->Execute(ctx);
	ctx.SetAssignValue(nullptr);
	return nullptr;
}
AstNodeAssignment* AstNodeAssignment::DeepCloneT() {
	AstNodeAssignment* newone = new AstNodeAssignment();

	newone->m_left	= m_left->DeepClone();
	newone->m_right = m_right->DeepClone();

	return newone;
}
