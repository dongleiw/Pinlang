#include "astnode_dereference.h"
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
llvm::Value* AstNodeDereference::Compile(CompileContext& cctx) {
	llvm::Value* value = m_value_expr->Compile(cctx); // 返回的可能是pointee value, 也可能是pointer to pointee value

	TypeInfoPointer* ti_pointer = dynamic_cast<TypeInfoPointer*>(g_typemgr.GetTypeInfo(m_result_typeid));
	TypeInfo*		 ti_pointee = g_typemgr.GetTypeInfo(ti_pointer->GetPointeeTid());

	llvm::Type* ir_type_pointee = ti_pointee->GetLLVMIRType(cctx);
	llvm::Type* ir_type_pointer = ti_pointer->GetLLVMIRType(cctx);

	if (value->getType() == ir_type_pointee) {
		// 返回的是value. 需要得到address
		// 有这种可能吗?
		panicf("bug");
	} else if (value->getType() == ir_type_pointer) {
		return value;
	} else {
		panicf("bug");
	}
}
AstNodeDereference* AstNodeDereference::DeepCloneT() {
	AstNodeDereference* newone = new AstNodeDereference();

	newone->m_value_expr = m_value_expr->DeepClone();

	return newone;
}
