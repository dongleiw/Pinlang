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
#include <bits/stdint-uintn.h>
#include <cassert>
#include <cstring>

Variable::Variable(TypeId tid) {
	m_tid	 = tid;
	m_is_tmp = true;

	memset((void*)&m_value, 0, sizeof(m_value));
	set_default_value();
}

Variable::Variable(int8_t value) {
	m_tid			  = TYPE_ID_INT8;
	m_value.value_int = value;
	m_is_tmp		  = true;
}
Variable::Variable(int16_t value) {
	m_tid			  = TYPE_ID_INT16;
	m_value.value_int = value;
	m_is_tmp		  = true;
}
Variable::Variable(int32_t value) {
	m_tid			  = TYPE_ID_INT32;
	m_value.value_int = value;
	m_is_tmp		  = true;
}
Variable::Variable(int64_t value) {
	m_tid			  = TYPE_ID_INT64;
	m_value.value_int = value;
	m_is_tmp		  = true;
}
Variable::Variable(uint8_t value) {
	m_tid			  = TYPE_ID_UINT8;
	m_value.value_int = value;
	m_is_tmp		  = true;
}
Variable::Variable(uint16_t value) {
	m_tid			  = TYPE_ID_UINT16;
	m_value.value_int = value;
	m_is_tmp		  = true;
}
Variable::Variable(uint32_t value) {
	m_tid			  = TYPE_ID_UINT32;
	m_value.value_int = value;
	m_is_tmp		  = true;
}
Variable::Variable(uint64_t value) {
	m_tid			  = TYPE_ID_UINT64;
	m_value.value_int = value;
	m_is_tmp		  = true;
}
int8_t Variable::GetValueInt8() const {
	assert(m_tid == TYPE_ID_INT8);
	return m_value.value_int;
}
int16_t Variable::GetValueInt16() const {
	assert(m_tid == TYPE_ID_INT16);
	return m_value.value_int;
}
int32_t Variable::GetValueInt32() const {
	assert(m_tid == TYPE_ID_INT32);
	return m_value.value_int;
}
int64_t Variable::GetValueInt64() const {
	assert(m_tid == TYPE_ID_INT64);
	return m_value.value_int;
}
uint8_t Variable::GetValueUInt8() const {
	assert(m_tid == TYPE_ID_UINT8);
	return m_value.value_int;
}
uint16_t Variable::GetValueUInt16() const {
	assert(m_tid == TYPE_ID_UINT16);
	return m_value.value_int;
}
uint32_t Variable::GetValueUInt32() const {
	assert(m_tid == TYPE_ID_UINT32);
	return m_value.value_int;
}
uint64_t Variable::GetValueUInt64() const {
	assert(m_tid == TYPE_ID_UINT64);
	return m_value.value_int;
}

Variable::Variable(float value) {
	m_tid				= TYPE_ID_FLOAT;
	m_value.value_float = value;
	m_is_tmp			= true;
}
Variable::Variable(bool value) {
	m_tid			   = TYPE_ID_BOOL;
	m_value.value_bool = value;
	m_is_tmp		   = true;
}
Variable::Variable(std::string value) {
	m_tid					= TYPE_ID_STR;
	m_value.value_str		= new TypeInfoStr::MemStructure();
	m_value.value_str->size = value.size();
	m_value.value_str->data = new uint8_t[value.size() + 1];
	memcpy((void*)m_value.value_str->data, (const void*)value.c_str(), value.size());
	m_value.value_str->data[value.size()] = '\0';
	m_is_tmp							  = true;
}
Variable::Variable(TypeId fn_tid, FunctionObj fnobj) {
	m_tid		  = fn_tid;
	m_value_fnobj = new FunctionObj(fnobj);
	m_is_tmp	  = true;
}
Variable::Variable(AstNodeConstraint* astnode) {
	m_tid			   = TYPE_ID_GENERIC_CONSTRAINT;
	m_value_constraint = astnode;
	m_is_tmp		   = true;
}
Variable::Variable(AstNodeComplexFnDef* astnode) {
	m_tid			   = TYPE_ID_COMPLEX_FN;
	m_value_complex_fn = astnode;
	m_is_tmp		   = true;
}
Variable::Variable(TypeId array_tid, std::vector<Variable*> array) {
	const TypeInfoArray* ti_array	 = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(array_tid));
	TypeId				 element_tid = ti_array->GetElementType();
	const TypeInfo*		 ti_element	 = g_typemgr.GetTypeInfo(element_tid);
	for (auto iter : array) {
		if (iter->GetTypeId() != element_tid) {
			panicf("bug");
		}
	}

	m_tid	 = array_tid;
	m_is_tmp = true;

	m_value.value_array		  = new TypeInfoArray::MemStructure();
	m_value.value_array->size = array.size();
	m_value.value_array->data = new uint8_t[array.size() * ti_element->GetMemSize()];
	uint8_t* ptr			  = m_value.value_array->data;
	for (auto iter : array) {
		memcpy((void*)ptr, (const void*)&iter->m_value, ti_element->GetMemSize());
		ptr += ti_element->GetMemSize();
	}
}
Variable* Variable::CreateTypeVariable(TypeId tid) {
	Variable* v			 = new Variable(TYPE_ID_TYPE);
	v->m_value.value_tid = tid;
	v->m_is_tmp			 = true;
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
		std::string field_name = std::string("f") + int_to_str(i);
		fields[field_name]	   = elements.at(i);
	}
	Variable* v = new Variable(tuple_tid);
	v->m_is_tmp = true;
	v->InitField(fields);
	return v;
}
std::string Variable::ToString() const {
	std::string ret;
	char		buf[128];
	switch (m_tid) {
	case TYPE_ID_TYPE:
		snprintf(buf, sizeof(buf) - 1, "type(%d:%s)", m_value.value_tid, GET_TYPENAME_C(m_value.value_tid));
		ret += buf;
		break;
	case TYPE_ID_INT8:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", int8_t(m_value.value_int));
		ret += buf;
		break;
	case TYPE_ID_INT16:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", int16_t(m_value.value_int));
		ret += buf;
		break;
	case TYPE_ID_INT32:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", int32_t(m_value.value_int));
		ret += buf;
		break;
	case TYPE_ID_INT64:
		snprintf(buf, sizeof(buf) - 1, "int(%ld)", int64_t(m_value.value_int));
		ret += buf;
		break;
	case TYPE_ID_STR:
		snprintf(buf, sizeof(buf) - 1, "str(%s)", m_value.value_str->data);
		ret += buf;
		break;
	case TYPE_ID_FLOAT:
		snprintf(buf, sizeof(buf) - 1, "float(%f)", m_value.value_float);
		ret += buf;
		break;
	case TYPE_ID_BOOL:
		snprintf(buf, sizeof(buf) - 1, "bool(%s)", m_value.value_bool == true ? "true" : "false");
		ret += buf;
		break;
	default:
		snprintf(buf, sizeof(buf) - 1, "unknown");
		ret += buf;
		break;
	}
	return ret;
}
TypeId Variable::GetValueTid() const {
	assert(m_tid == TYPE_ID_TYPE);
	return m_value.value_tid;
}
float Variable::GetValueFloat() const {
	assert(m_tid == TYPE_ID_FLOAT);
	return m_value.value_float;
}
bool Variable::GetValueBool() const {
	assert(m_tid == TYPE_ID_BOOL);
	return m_value.value_bool;
}
char* Variable::GetValueStr() {
	assert(m_tid == TYPE_ID_STR);
	return (char*)m_value.value_str->data;
}
int Variable::GetValueStrSize() const {
	assert(m_tid == TYPE_ID_STR);
	return m_value.value_str->size;
}
FunctionObj* Variable::GetValueFunctionObj() const {
	assert(g_typemgr.GetTypeInfo(m_tid)->IsFn());
	return m_value_fnobj;
}
AstNodeConstraint* Variable::GetValueConstraint() const {
	assert(m_tid == TYPE_ID_GENERIC_CONSTRAINT);
	return m_value_constraint;
}
AstNodeComplexFnDef* Variable::GetValueComplexFn() const {
	assert(m_tid == TYPE_ID_COMPLEX_FN);
	return m_value_complex_fn;
}
const int Variable::GetValueArraySize() const {
	assert(g_typemgr.GetTypeInfo(m_tid)->IsArray());
	return m_value.value_array->size;
}
Variable* Variable::GetValueArrayElement(int idx) {
	assert(0 <= idx && idx < GetValueArraySize());

	TypeInfoArray* ti_array	  = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(m_tid));
	TypeInfo*	   ti_element = g_typemgr.GetTypeInfo(ti_array->GetElementType());
	Variable*	   element	  = new Variable(ti_element->GetTypeId());
	element->SetTmp(false);
	memcpy((void*)&element->m_value, (const void*)(m_value.value_array->data + idx * ti_element->GetMemSize()), ti_element->GetMemSize());
	return element;
}
void Variable::SetValueArrayElement(int idx, Variable* element) {
	assert(0 <= idx && idx < GetValueArraySize());

	TypeInfoArray* ti_array	  = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(m_tid));
	TypeInfo*	   ti_element = g_typemgr.GetTypeInfo(ti_array->GetElementType());
	memcpy((void*)(m_value.value_array->data + idx * ti_element->GetMemSize()), (const void*)&element->m_value, ti_element->GetMemSize());
}
Variable* Variable::GetFieldValue(std::string field_name) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	assert(ti->IsTuple() || ti->IsClass());

	for (auto field : ti->GetField()) {
		if (field.name == field_name) {
			TypeInfo* ti_field	  = g_typemgr.GetTypeInfo(field.tid);
			Variable* field_value = new Variable(field.tid);
			memcpy((void*)&field_value->m_value, (const void*)(m_value.value_fields + field.mem_offset), ti_field->GetMemSize());
			return field_value;
		}
	}
	panicf("field[%s] not exists", field_name.c_str());
	return nullptr;
}
void Variable::SetFieldValue(std::string field_name, Variable* v) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	assert(ti->IsTuple() || ti->IsClass());

	for (auto field : ti->GetField()) {
		if (field.name == field_name) {
			TypeInfo* ti_field = g_typemgr.GetTypeInfo(field.tid);
			memcpy((void*)(m_value.value_fields + field.mem_offset), (void*)&v->m_value, ti_field->GetMemSize());
			return;
		}
	}
	panicf("field[%s] not exists", field_name.c_str());
}
void Variable::Assign(Variable* tmp) {
	assert(!IsTmp());
	assert(m_tid == tmp->GetTypeId());

	TypeId tmp_tid = m_tid;

	*this = *tmp;

	m_tid	 = tmp_tid;
	m_is_tmp = false;
}
void Variable::InitField(std::map<std::string, Variable*> fields) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	assert(ti->IsTuple() || ti->IsClass());
	assert(ti->GetField().size() == fields.size());

	m_value.value_fields = new uint8_t[ti->GetMemSize()];
	for (auto field : ti->GetField()) {
		TypeInfo* ti_field = g_typemgr.GetTypeInfo(field.tid);
		auto	  found	   = fields.find(field.name);
		assert(found != fields.end());
		memcpy((void*)(m_value.value_fields + field.mem_offset), (const void*)&found->second->m_value, ti_field->GetMemSize());
	}
}
void Variable::set_default_value() {
	const TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	if (ti->IsTuple() || ti->IsClass()) {
		m_value.value_fields = new uint8_t[ti->GetMemSize()];
		for (auto field : ti->GetField()) {
			TypeInfo* ti_field		= g_typemgr.GetTypeInfo(field.tid);
			Variable* default_value = new Variable(field.tid);
			memcpy((void*)(m_value.value_fields + field.mem_offset), (const void*)&default_value->m_value, ti_field->GetMemSize());
		}
	} else if (ti->IsArray()) {
		assert(m_value.value_array == nullptr);
		m_value.value_array = new TypeInfoArray::MemStructure();
	} else if (ti->IsFn()) {
		// panicf("function can not have default value");
		m_value_fnobj = nullptr;
	} else {
		if (is_integer_type(m_tid)) {
			m_value.value_int = 0;
		} else {
			switch (m_tid) {
			case TYPE_ID_TYPE:
				m_value.value_tid = TYPE_ID_NONE;
				break;
			case TYPE_ID_FLOAT:
				m_value.value_float = 0.0;
				break;
			case TYPE_ID_BOOL:
				m_value.value_bool = false;
				break;
			case TYPE_ID_STR:
				m_value.value_str		   = new TypeInfoStr::MemStructure();
				m_value.value_str->data	   = new uint8_t[1];
				m_value.value_str->data[0] = '\0';
				break;
			default:
				panicf("unknown type[%d:%s]", m_tid, GET_TYPENAME_C(m_tid));
			}
		}
	}
}
