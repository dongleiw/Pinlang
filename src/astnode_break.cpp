#include "astnode_break.h"
#include "compile_context.h"
#include "define.h"
#include "log.h"
#include "type_mgr.h"
#include "verify_context.h"

AstNodeBreak::AstNodeBreak() {
}

VerifyContextResult AstNodeBreak::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	if (!IsInFor()) {
		panicf("break can only be used in for statement");
	}
	VerifyContextResult vr_result;
	return vr_result;
}
Variable* AstNodeBreak::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->SetBreaked(true);
	return nullptr;
}
AstNodeBreak* AstNodeBreak::DeepCloneT() {
	return new AstNodeBreak();
}
CompileResult AstNodeBreak::Compile(CompileContext& cctx) {
	assert(cctx.GetBreakBlock() != nullptr);
	IRB.CreateBr(cctx.GetBreakBlock());
	return CompileResult();
}
