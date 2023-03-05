#include "astnode_reference.h"
#include "type.h"
#include "type_mgr.h"
#include "type_pointer.h"

AstNodeReference::AstNodeReference(AstNode* expr) {
	m_expr					= expr;
	m_compile_to_left_value = false;
}
VerifyContextResult AstNodeReference::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	verify_begin();

	m_compile_to_left_value = vparam.ExpectLeftValue();

	VerifyContextResult vr_value = m_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(false));

	TypeInfo* ti = g_typemgr.GetTypeInfo(vr_value.GetResultTypeId());
	if (!ti->IsPointer()) {
		panicf("type[%d:%s] is not pointer. cannot do reference operation", vr_value.GetResultTypeId(), GET_TYPENAME_C(vr_value.GetResultTypeId()));
	}
	TypeInfoPointer* ti_pointer = dynamic_cast<TypeInfoPointer*>(ti);

	m_result_typeid = ti_pointer->GetPointeeTid();

	verify_end();

	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeReference::Execute(ExecuteContext& ctx) {
	panicf("not implemented");
}
llvm::Value* AstNodeReference::Compile(CompileContext& cctx) {
	llvm::Value* value = m_expr->Compile(cctx); // 返回的可能是pointee value, 也可能是pointer to pointee value

	TypeInfo* ti_pointee = g_typemgr.GetTypeInfo(m_result_typeid);

	llvm::Type* ir_type_pointee = ti_pointee->GetLLVMIRType(cctx);

	if (m_compile_to_left_value) {
		return value;
	} else {
		return IRB.CreateLoad(ir_type_pointee, value);
	}
}
AstNodeReference* AstNodeReference::DeepCloneT() {
	AstNodeReference* newone = new AstNodeReference();

	newone->m_expr = m_expr->DeepClone();

	return newone;
}
