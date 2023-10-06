#include "astnode_assignment.h"
#include "astnode.h"
#include "astnode_complex_fndef.h"
#include "compile_context.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <cassert>
#include <llvm-12/llvm/Support/Alignment.h>

AstNodeAssignment::AstNodeAssignment(AstNode* left, AstNode* right) {
	m_left	= left;
	m_right = right;

	m_left->SetParent(this);
	m_right->SetParent(this);
}
VerifyContextResult AstNodeAssignment::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	log_debug("verify assigment");

	VerifyContextResult vr_left = m_left->Verify(ctx, VerifyContextParam().SetExepectLeftValue(true));
	if (vr_left.GetResultTypeId() == TYPE_ID_NONE) {
		panicf("type of left expr in assignment is none");
	}
	if (vr_left.IsTmp()) {
		panicf("cannot assign temporary value");
	}

	VerifyContextResult vr_right = m_right->Verify(ctx, VerifyContextParam().SetExpectResultTid(vr_left.GetResultTypeId()));
	if (vr_right.GetResultTypeId() != vr_left.GetResultTypeId()) {
		panicf("type not match in assignment: left[%d:%s] right[%d:%s]", vr_left.GetResultTypeId(), GET_TYPENAME_C(vr_left.GetResultTypeId()),
			   vr_right.GetResultTypeId(), GET_TYPENAME_C(vr_right.GetResultTypeId()));
	}

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeAssignment::Execute(ExecuteContext& ctx) {
	Variable* v_right = m_right->Execute(ctx);
	ctx.SetAssignValue(v_right);
	m_left->Execute(ctx);
	ctx.SetAssignValue(nullptr);
	return nullptr;
}
AstNodeAssignment* AstNodeAssignment::DeepCloneT() {
	AstNodeAssignment* newone = new AstNodeAssignment(m_left->DeepClone(), m_right->DeepClone());
	newone->Copy(*this);
	return newone;
}
CompileResult AstNodeAssignment::Compile(CompileContext& cctx) {
	CompileResult cr_left_value	 = m_left->Compile(cctx);
	CompileResult cr_right_value = m_right->Compile(cctx);

	llvm::Type* ir_type_left  = cr_left_value.GetResult()->getType();
	llvm::Type* ir_type_right = cr_right_value.GetResult()->getType();

	assert(g_typemgr.GetTypeInfo(m_right->GetResultTypeId())->GetLLVMIRType(cctx) == ir_type_right);
	assert(ir_type_left == ir_type_right->getPointerTo());

	llvm::Value* result = IRB.CreateStore(cr_right_value.GetResult(), cr_left_value.GetResult());
	return CompileResult().SetResult(result);
}
