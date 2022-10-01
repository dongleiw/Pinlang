#include "astnode_access_array_element.h"
#include "define.h"
#include "log.h"
#include "type_array.h"
#include "type_mgr.h"
#include "variable.h"

VerifyContextResult AstNodeAccessArrayElement::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify access array element");

	TypeId	  array_tid = m_array_expr->Verify(ctx, VerifyContextParam()).GetResultTypeId();
	TypeInfo* ti		= g_typemgr.GetTypeInfo(array_tid);
	if (!ti->IsArray()) {
		panicf("not array");
	}
	TypeInfoArray* ti_array = dynamic_cast<TypeInfoArray*>(ti);

	TypeId index_tid = m_index_expr->Verify(ctx, VerifyContextParam()).GetResultTypeId();
	if (index_tid != TYPE_ID_INT) {
		panicf("index type is not int");
	}

	m_result_typeid = ti_array->GetElementType();
	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeAccessArrayElement::Execute(ExecuteContext& ctx) {
	Variable* v_array = m_array_expr->Execute(ctx);
	Variable* v_index = m_index_expr->Execute(ctx);

	std::vector<Variable*> elements = v_array->GetValueArray();
	int					   index	= v_index->GetValueInt();
	if (index < 0 || index >= elements.size()) {
		panicf("invalid index");
	}
	return elements.at(index);
}
AstNodeAccessArrayElement* AstNodeAccessArrayElement::DeepCloneT() {
	AstNodeAccessArrayElement* newone = new AstNodeAccessArrayElement();

	newone->m_array_expr = m_array_expr->DeepClone();
	newone->m_index_expr = m_index_expr->DeepClone();

	return newone;
}
