#include "astnode_init_array.h"
#include "astnode_type.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_array.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

VerifyContextResult AstNodeInitArray::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify init array");

	m_result_typeid = m_array_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();

	TypeInfoArray* ti				 = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(m_result_typeid));
	TypeId		   array_element_tid = ti->GetElementType();

	for (auto iter : m_init_values) {
		VerifyContextResult vr = iter->Verify(ctx, VerifyContextParam().SetResultTid(array_element_tid));
		if (vr.GetResultTypeId() != array_element_tid) {
			panicf("type of array init value [%d:%s] != [%d:%s]", vr.GetResultTypeId(), GET_TYPENAME_C(vr.GetResultTypeId()), array_element_tid, GET_TYPENAME_C(array_element_tid));
		}
	}
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeInitArray::Execute(ExecuteContext& ctx) {
	std::vector<Variable*> elements;
	for (auto iter : m_init_values) {
		elements.push_back(iter->Execute(ctx));
	}
	return new Variable(m_result_typeid, elements);
}
AstNodeInitArray* AstNodeInitArray::DeepCloneT() {
	AstNodeInitArray* newone = new AstNodeInitArray();
	newone->m_array_type	 = m_array_type->DeepCloneT();
	for (auto iter : m_init_values) {
		newone->m_init_values.push_back(iter->DeepClone());
	}
	return newone;
}
