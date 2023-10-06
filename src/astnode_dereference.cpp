#include "astnode_dereference.h"
#include "compile_context.h"
#include "define.h"
#include "type.h"
#include "type_mgr.h"
#include "type_pointer.h"

AstNodeDereference::AstNodeDereference(AstNode* value_expr) {
	m_value_expr = value_expr;
}
/*
 * `*expr`
 *		`expr`的类型必须是指针, 假设为*T
 *		`*expr`的类型是T
 *		如果被要求返回rvalue:
 *			要求`expr`返回rvalue. expr是指针, 所以返回的是一个内存地址
 *			从这个内存地址中load数据到register, 然后返回这个值
 *		如果被要求返回lvalue:
 *			要求`expr`返回rvalue. expr是指针, 所以返回的是一个内存地址
 *			返回这个内存地址
 *			
 */
VerifyContextResult AstNodeDereference::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	VerifyContextResult vr_value = m_value_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(false));

	TypeInfo* ti = g_typemgr.GetTypeInfo(vr_value.GetResultTypeId());
	if (!ti->IsPointer()) {
		panicf("[%d:%s] is not pointer. cannot dereference", vr_value.GetResultTypeId(), ti->GetName().c_str());
	}
	TypeInfoPointer* ti_pointer = dynamic_cast<TypeInfoPointer*>(ti);
	m_result_typeid				= ti_pointer->GetPointeeTid();
	m_compile_to_left_value		= vparam.ExpectLeftValue();
	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeDereference::Execute(ExecuteContext& ctx) {
	panicf("not implemented");
}
CompileResult AstNodeDereference::Compile(CompileContext& cctx) {
	CompileResult cr = m_value_expr->Compile(cctx);

	TypeInfo*	ti_pointee		= g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Type* ir_type_pointee = ti_pointee->GetLLVMIRType(cctx);
	llvm::Type* ir_type_pointer = ir_type_pointee->getPointerTo();
	assert(cr.GetResult()->getType() == ir_type_pointer);

	if (m_compile_to_left_value) {
		return CompileResult().SetResult(cr.GetResult());
	} else {
		return CompileResult().SetResult(IRB.CreateLoad(ir_type_pointee, cr.GetResult()));
	}
}
AstNodeDereference* AstNodeDereference::DeepCloneT() {
	AstNodeDereference* newone = new AstNodeDereference();
	newone->Copy(*this);

	newone->m_value_expr = m_value_expr->DeepClone();

	return newone;
}
