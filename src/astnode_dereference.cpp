#include "astnode_dereference.h"
#include "compile_context.h"
#include "type.h"
#include "type_mgr.h"
#include "type_pointer.h"

AstNodeDereference::AstNodeDereference(AstNode* value_expr) {
	m_value_expr = value_expr;
}
VerifyContextResult AstNodeDereference::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	verify_begin();

	VerifyContextResult vr_value = m_value_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(true));

	bool added;
	m_result_typeid = g_typemgr.GetOrAddTypePointer(ctx, vr_value.GetResultTypeId(), added);

	verify_end();

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeDereference::Execute(ExecuteContext& ctx) {
	panicf("not implemented");
}
CompileResult AstNodeDereference::Compile(CompileContext& cctx) {
	CompileResult cr = m_value_expr->Compile(cctx);

	TypeInfoPointer* ti_pointer = dynamic_cast<TypeInfoPointer*>(g_typemgr.GetTypeInfo(m_result_typeid));

	llvm::Type* ir_type_pointer = ti_pointer->GetLLVMIRType(cctx);
	assert(cr.GetResult()->getType() == ir_type_pointer);
	return CompileResult().SetResult(cr.GetResult());
}
AstNodeDereference* AstNodeDereference::DeepCloneT() {
	AstNodeDereference* newone = new AstNodeDereference();

	newone->m_value_expr = m_value_expr->DeepClone();

	return newone;
}
