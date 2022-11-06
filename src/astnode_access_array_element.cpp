#include "astnode_access_array_element.h"
#include "astnode_constraint.h"
#include "define.h"
#include "log.h"
#include "type_array.h"
#include "type_mgr.h"
#include "variable.h"

VerifyContextResult AstNodeAccessArrayElement::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify access array element");

	if (vparam.ExpectLeftValue()) {
	}

	TypeId	  obj_tid = m_array_expr->Verify(ctx, VerifyContextParam()).GetResultTypeId();
	TypeInfo* ti	  = g_typemgr.GetTypeInfo(obj_tid);

	TypeId index_tid = m_index_expr->Verify(ctx, VerifyContextParam()).GetResultTypeId();
	if (index_tid != TYPE_ID_INT32) {
		panicf("index type is not int");
	}

	std::vector<MethodIndex> method_indexs = ti->GetConstraintMethod(ctx, "Index", "index", std::vector<TypeId>{index_tid});
	if (method_indexs.empty()) {
		panicf("type[%d:%s] not implement constraint Index", obj_tid, GET_TYPENAME_C(obj_tid));
	} else if (method_indexs.size() > 1) {
		panicf("type[%d:%s] has multiple method[index] of constraint[Index]", obj_tid, GET_TYPENAME_C(obj_tid));
	} else {
	}

	MethodIndex method_index = method_indexs.at(0);
	m_fn_addr				 = ti->GetMethodByIdx(method_index);
	m_result_typeid			 = ctx.GetFnTable().GetFnReturnTypeId(m_fn_addr);

	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeAccessArrayElement::Execute(ExecuteContext& ctx) {
	Variable* assign_value = ctx.GetAssignValue();
	ctx.SetAssignValue(nullptr);

	Variable* v_array = m_array_expr->Execute(ctx);
	Variable* v_index = m_index_expr->Execute(ctx);

	if (assign_value != nullptr) {
		v_array->SetValueArrayElement(v_index->GetValueInt32(), assign_value);
		return nullptr;
	} else {
		std::vector<Variable*> args{v_index};
		return ctx.GetFnTable().CallFn(m_fn_addr, ctx, v_array, args);
	}
}
AstNodeAccessArrayElement* AstNodeAccessArrayElement::DeepCloneT() {
	AstNodeAccessArrayElement* newone = new AstNodeAccessArrayElement();

	newone->m_array_expr = m_array_expr->DeepClone();
	newone->m_index_expr = m_index_expr->DeepClone();

	return newone;
}
