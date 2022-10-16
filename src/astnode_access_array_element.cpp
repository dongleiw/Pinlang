#include "astnode_access_array_element.h"
#include "astnode_constraint.h"
#include "define.h"
#include "log.h"
#include "type_array.h"
#include "type_mgr.h"
#include "variable.h"
#include "function.h"

VerifyContextResult AstNodeAccessArrayElement::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify access array element");

	TypeId	  obj_tid = m_array_expr->Verify(ctx, VerifyContextParam()).GetResultTypeId();
	TypeInfo* ti	  = g_typemgr.GetTypeInfo(obj_tid);

	TypeId index_tid = m_index_expr->Verify(ctx, VerifyContextParam()).GetResultTypeId();
	if (index_tid != TYPE_ID_INT) {
		panicf("index type is not int");
	}

	std::vector<MethodIndex> method_indexs = ti->GetConstraintMethod(ctx, "Index", "index", std::vector<TypeId>{index_tid});
	if (method_indexs.empty()) {
		panicf("type[%d:%s] not implement constraint Index", obj_tid, GET_TYPENAME_C(obj_tid));
	} else if (method_indexs.size() > 1) {
		panicf("type[%d:%s] has multiple method[index] of constraint[Index]", obj_tid, GET_TYPENAME_C(obj_tid));
	} else {
	}

	m_method_index= method_indexs.at(0);
	Function*	f		   = ti->GetMethodByIdx(m_method_index);
	m_result_typeid = f->GetReturnTypeId();

	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeAccessArrayElement::Execute(ExecuteContext& ctx) {
	Variable* v_array = m_array_expr->Execute(ctx);
	Variable* v_index = m_index_expr->Execute(ctx);
	std::vector<Variable*> args{v_index};
	return v_array->CallMethod(ctx, m_method_index, args);
}
AstNodeAccessArrayElement* AstNodeAccessArrayElement::DeepCloneT() {
	AstNodeAccessArrayElement* newone = new AstNodeAccessArrayElement();

	newone->m_array_expr = m_array_expr->DeepClone();
	newone->m_index_expr = m_index_expr->DeepClone();

	return newone;
}
