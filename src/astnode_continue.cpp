#include "astnode_continue.h"
#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "log.h"
#include "type_mgr.h"
#include "verify_context.h"

AstNodeContinue::AstNodeContinue() {
}

VerifyContextResult AstNodeContinue::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	if (!IsInFor()) {
		panicf("break can only be used in for statement");
	}
	VerifyContextResult vr_result;
	return vr_result;
}
Variable* AstNodeContinue::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->SetBreaked(true);
	return nullptr;
}
AstNodeContinue* AstNodeContinue::DeepCloneT() {
	AstNodeContinue* newone = new AstNodeContinue();
	newone->Copy(*this);
	return newone;
}
CompileResult AstNodeContinue::Compile(CompileContext& cctx) {
	assert(cctx.GetContinueBlock() != nullptr);
	IRB.CreateBr(cctx.GetContinueBlock());
	return CompileResult();
}
