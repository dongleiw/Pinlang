#include "astnode_assignment.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <llvm-12/llvm/Support/Alignment.h>

VerifyContextResult AstNodeAssignment::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify assigment");

	VerifyContextResult vr_left = m_left->Verify(ctx, VerifyContextParam().SetExepectLeftValue(true));
	if (vr_left.GetResultTypeId() == TYPE_ID_NONE) {
		panicf("type of left expr in assignment is none");
	}
	if (vr_left.IsTmp()) {
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
	Variable* v_right = m_right->Execute(ctx);
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
llvm::Value* AstNodeAssignment::Compile(CompileContext& cctx) {
	llvm::Value* left_value	 = m_left->Compile(cctx);
	llvm::Value* right_value = m_right->Compile(cctx);

	llvm::Type* ir_type_left  = left_value->getType();
	llvm::Type* ir_type_right = right_value->getType();

	if (!ir_type_left->isPointerTy()) {
		// left-expr结果必须是一个地址
		panicf("left expr is not pointer");
	}
	if (ir_type_left == ir_type_right) {
		// right-expr的结果是pointer
		llvm::Value* loaded_right_value = IRB.CreateLoad(ir_type_right, right_value);
		return IRB.CreateStore(loaded_right_value, left_value);
	} else if (ir_type_left == ir_type_right->getPointerTo()) {
		return IRB.CreateStore(right_value, left_value);
	} else {
		panicf("bug");
	}
}
