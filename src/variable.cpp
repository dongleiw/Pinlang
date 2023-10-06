#include "variable.h"
#include "define.h"
#include "function_obj.h"
#include "log.h"
#include "type.h"
#include "type_array.h"
#include "type_mgr.h"
#include "type_str.h"
#include "type_tuple.h"
#include "utils.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <map>

Variable::Variable(TypeId tid) {
	construct_init(tid, nullptr);
}

Variable::Variable(int8_t value) {
	construct_init(TYPE_ID_INT8, (const void*)&value);
}
Variable::Variable(int16_t value) {
	construct_init(TYPE_ID_INT16, (const void*)&value);
}
Variable::Variable(int32_t value) {
	construct_init(TYPE_ID_INT32, (const void*)&value);
}
Variable::Variable(int64_t value) {
	construct_init(TYPE_ID_INT64, (const void*)&value);
}
Variable::Variable(uint8_t value) {
	construct_init(TYPE_ID_UINT8, (const void*)&value);
}
Variable::Variable(uint16_t value) {
	construct_init(TYPE_ID_UINT16, (const void*)&value);
}
Variable::Variable(uint32_t value) {
	construct_init(TYPE_ID_UINT32, (const void*)&value);
}
Variable::Variable(uint64_t value) {
	construct_init(TYPE_ID_UINT64, (const void*)&value);
	m_value_u64 = value;
}

Variable::Variable(float value) {
	construct_init(TYPE_ID_FLOAT32, (const void*)&value);
}
Variable::Variable(double value) {
	construct_init(TYPE_ID_FLOAT64, (const void*)&value);
}
Variable::Variable(bool value) {
	construct_init(TYPE_ID_BOOL, (const void*)&value);
}
Variable::Variable(std::string value) {
	construct_init(TYPE_ID_STR, nullptr);
}
Variable::Variable(TypeId fn_tid, FunctionObj fnobj) {
	construct_init(fn_tid, (const void*)&fnobj);
}
Variable::Variable(AstNodeConstraint* astnode) {
	m_tid			   = TYPE_ID_GENERIC_CONSTRAINT;
	m_value_constraint = astnode;
}
Variable::Variable(AstNodeComplexFnDef* astnode) {
	m_tid			   = TYPE_ID_COMPLEX_FN;
	m_value_complex_fn = astnode;
}
Variable::Variable(TypeId array_tid, std::vector<Variable*> array) {
	construct_init(array_tid, nullptr);

	const TypeInfoArray* ti_array	 = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(array_tid));
	TypeId				 element_tid = ti_array->GetElementType();
	const TypeInfo*		 ti_element	 = g_typemgr.GetTypeInfo(element_tid);
	for (auto iter : array) {
		if (iter->GetTypeId() != element_tid) {
			panicf("bug");
		}
	}
}
Variable* Variable::CreateTypeVariable(TypeId tid) {
	Variable* v	   = new Variable(TYPE_ID_TYPE);
	v->m_value_tid = tid;
	return v;
}
Variable* Variable::CreateTypeTuple(TypeId tuple_tid, std::vector<Variable*> elements) {
	TypeInfoTuple* ti = dynamic_cast<TypeInfoTuple*>(g_typemgr.GetTypeInfo(tuple_tid));
	if (!ti->IsTuple() || ti->GetElementTids().size() != elements.size()) {
		panicf("bug");
	}
	std::map<std::string, Variable*> fields;
	for (size_t i = 0; i < elements.size(); i++) {
		if (ti->GetElementTids().at(i) != elements.at(i)->GetTypeId()) {
			panicf("bug");
		}
		std::string field_name = std::string("f") + to_str(i);
		fields[field_name]	   = elements.at(i);
	}
	Variable* v = new Variable(tuple_tid);
	return v;
}
TypeId Variable::GetValueTid() const {
	assert(m_tid == TYPE_ID_TYPE);
	return m_value_tid;
}
uint64_t Variable::GetValueU64() const {
	assert(m_tid == TYPE_ID_UINT64);
	return m_value_u64;
}
AstNodeConstraint* Variable::GetValueConstraint() const {
	assert(m_tid == TYPE_ID_GENERIC_CONSTRAINT);
	return m_value_constraint;
}
AstNodeComplexFnDef* Variable::GetValueComplexFn() const {
	assert(m_tid == TYPE_ID_COMPLEX_FN);
	return m_value_complex_fn;
}
void Variable::construct_init(TypeId tid, const void* data) {
	m_tid = tid;
}
