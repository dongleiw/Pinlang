#include "astnode_reference.h"
#include "compile_context.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "type_pointer.h"

AstNodeReference::AstNodeReference(AstNode* expr) {
	m_expr = expr;
}
/*
 * `&expr`
 *		`expr`的类型假设为T
 *		`&expr`的类型是*T
 *		如果被要求返回rvalue:
 *			要求`expr`返回lvalue
 *			return `expr`返回的lvalue
 *		如果被要求返回lvalue:
 *			错误. 不允许返回lvalue
 *
 */
VerifyContextResult AstNodeReference::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	if (vparam.ExpectLeftValue()) {
		panicf("&expr cannot be lvalue");
	}

	VerifyContextResult vr_value = m_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(true));

	m_compile_to_left_value = vparam.ExpectLeftValue();
	m_result_typeid			= g_typemgr.GetOrAddTypePointer(ctx, vr_value.GetResultTypeId());

	return VerifyContextResult(m_result_typeid).SetTmp(true);
}
Variable* AstNodeReference::Execute(ExecuteContext& ctx) {
	panicf("not implemented");
}
CompileResult AstNodeReference::Compile(CompileContext& cctx) {
	if (m_compile_to_left_value) {
		panicf("&expr cannot be lvalue");
	}

	CompileResult cr_value = m_expr->Compile(cctx);

	TypeInfo* ti = g_typemgr.GetTypeInfo(m_result_typeid);

	assert(cr_value.GetResult()->getType() == ti->GetLLVMIRType(cctx));

	return CompileResult().SetResult(cr_value.GetResult());
}
AstNodeReference* AstNodeReference::DeepCloneT() {
	AstNodeReference* newone = new AstNodeReference();
	newone->Copy(*this);

	newone->m_expr = m_expr->DeepClone();

	return newone;
}
