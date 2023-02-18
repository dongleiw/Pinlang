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
	set_default_value();
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
}
int8_t Variable::GetValueInt8() const {
	assert(m_tid == TYPE_ID_INT8);
	return *(int8_t*)m_data; // value type
}
int16_t Variable::GetValueInt16() const {
	assert(m_tid == TYPE_ID_INT16);
	return *(int16_t*)m_data; // value type
}
int32_t Variable::GetValueInt32() const {
	assert(m_tid == TYPE_ID_INT32);
	return *(int32_t*)m_data; // value type
}
int64_t Variable::GetValueInt64() const {
	assert(m_tid == TYPE_ID_INT64);
	return *(int64_t*)m_data; // value type
}
uint8_t Variable::GetValueUInt8() const {
	assert(m_tid == TYPE_ID_UINT8);
	return *(uint8_t*)m_data; // value type
}
uint16_t Variable::GetValueUInt16() const {
	assert(m_tid == TYPE_ID_UINT16);
	return *(uint16_t*)m_data; // value type
}
uint32_t Variable::GetValueUInt32() const {
	assert(m_tid == TYPE_ID_UINT32);
	return *(uint32_t*)m_data; // value type
}
uint64_t Variable::GetValueUInt64() const {
	assert(m_tid == TYPE_ID_UINT64);
	return *(uint64_t*)m_data; // value type
}

Variable::Variable(float value) {
	construct_init(TYPE_ID_FLOAT, (const void*)&value);
}
Variable::Variable(bool value) {
	construct_init(TYPE_ID_BOOL, (const void*)&value);
}
Variable::Variable(std::string value) {
	construct_init(TYPE_ID_STR, nullptr);

	TypeInfoStr::MemStructure* st = (TypeInfoStr::MemStructure*)m_data;
	st->size					  = value.size();
	st->data					  = new uint8_t[value.size() + 1]; // 增加一个null结尾
	memcpy((void*)st->data, (const void*)value.c_str(), value.size());
	st->data[value.size()] = '\0';
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

	uint8_t* array_data = nullptr;
	if (ti_array->IsStaticSize()) {
		array_data = m_data;
	} else {
		array_data = new uint8_t[array.size() * ti_element->GetMemSize()];
		memcpy(m_data, (const void*)&array_data, sizeof(void*)); // m_data存储数组实际数据地址
	}

	for (auto iter : array) {
		memcpy((void*)array_data, (const void*)iter->m_data, ti_element->GetMemSize());
		array_data += ti_element->GetMemSize();
	}
}
Variable* Variable::CreateTypeVariable(TypeId tid) {
	Variable* v = new Variable(TYPE_ID_TYPE);
	memcpy(v->m_data, (const void*)&tid, sizeof(TypeId));
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
	v->InitField(fields);
	return v;
}
std::string Variable::ToString() const {
	std::string ret;
	char		buf[128];
	switch (m_tid) {
	case TYPE_ID_TYPE:
		snprintf(buf, sizeof(buf) - 1, "type(%d:%s)", *(TypeId*)m_data, GET_TYPENAME_C(*(TypeId*)m_data));
		snprintf(buf, sizeof(buf) - 1, "type(%d:%s)", GetValueTid(), GET_TYPENAME_C(GetValueTid()));
		ret += buf;
		break;
	case TYPE_ID_INT8:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", GetValueInt8());
		ret += buf;
		break;
	case TYPE_ID_INT16:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", GetValueInt16());
		ret += buf;
		break;
	case TYPE_ID_INT32:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", GetValueInt32());
		ret += buf;
		break;
	case TYPE_ID_INT64:
		snprintf(buf, sizeof(buf) - 1, "int(%ld)", GetValueInt64());
		ret += buf;
		break;
	case TYPE_ID_STR:
		snprintf(buf, sizeof(buf) - 1, "str(%s)", GetValueStr());
		ret += buf;
		break;
	case TYPE_ID_FLOAT:
		snprintf(buf, sizeof(buf) - 1, "float(%f)", GetValueFloat());
		ret += buf;
		break;
	case TYPE_ID_BOOL:
		snprintf(buf, sizeof(buf) - 1, "bool(%s)", GetValueBool() == true ? "true" : "false");
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
	return *(TypeId*)m_data;
}
float Variable::GetValueFloat() const {
	assert(m_tid == TYPE_ID_FLOAT);
	return *(float*)m_data;
}
bool Variable::GetValueBool() const {
	assert(m_tid == TYPE_ID_BOOL);
	return *(bool*)m_data;
}
char* Variable::GetValueStr() {
	assert(m_tid == TYPE_ID_STR);
	return (char*)((TypeInfoStr::MemStructure*)m_data)->data;
}
const char* Variable::GetValueStr() const {
	assert(m_tid == TYPE_ID_STR);
	return (const char*)((TypeInfoStr::MemStructure*)m_data)->data;
}
int Variable::GetValueStrSize() const {
	assert(m_tid == TYPE_ID_STR);
	return ((TypeInfoStr::MemStructure*)m_data)->size;
}
FunctionObj* Variable::GetValueFunctionObj() const {
	assert(g_typemgr.GetTypeInfo(m_tid)->IsFn());
	return (FunctionObj*)m_data;
}
AstNodeConstraint* Variable::GetValueConstraint() const {
	assert(m_tid == TYPE_ID_GENERIC_CONSTRAINT);
	return m_value_constraint;
}
AstNodeComplexFnDef* Variable::GetValueComplexFn() const {
	assert(m_tid == TYPE_ID_COMPLEX_FN);
	return m_value_complex_fn;
}
Variable* Variable::GetValueArrayElement(int idx) {
	TypeInfoArray* ti_array	  = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(m_tid));
	TypeInfo*	   ti_element = g_typemgr.GetTypeInfo(ti_array->GetElementType());
	Variable*	   element	  = new Variable(ti_element->GetTypeId());
	const uint8_t* array_data = nullptr;
	if (ti_array->IsStaticSize()) {
		array_data = m_data;
	} else {
		uint64_t array_data_address = *(const uint64_t*)m_data;
		array_data					= (const uint8_t*)array_data_address;
	}
	memcpy((void*)element->m_data, (const void*)(array_data + idx * ti_element->GetMemSize()), ti_element->GetMemSize());
	return element;
}
void Variable::SetValueArrayElement(int idx, Variable* element) {
	TypeInfoArray* ti_array	  = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(m_tid));
	TypeInfo*	   ti_element = g_typemgr.GetTypeInfo(ti_array->GetElementType());

	uint8_t* array_data = nullptr;
	if (ti_array->IsStaticSize()) {
		array_data = m_data;
	} else {
		uint64_t array_data_address = *(uint64_t*)m_data;
		array_data					= (uint8_t*)array_data_address;
	}
	memcpy((void*)(array_data + idx * ti_element->GetMemSize()), (const void*)element->m_data, ti_element->GetMemSize());
}
Variable* Variable::GetFieldValue(std::string field_name) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	assert(ti->IsTuple() || ti->IsClass());

	for (auto field : ti->GetField()) {
		if (field.name == field_name) {
			TypeInfo* ti_field	  = g_typemgr.GetTypeInfo(field.tid);
			Variable* field_value = new Variable(field.tid);
			// 这里生成了一个临时变量. 如果克隆一份数据, 那么对临时变量的修改就无法影响该对象的该字段的值
			// 因此这里将m_data指向该对象的该字段的值.
			field_value->m_data = m_data + field.mem_offset;
			//memcpy((void*)field_value->m_data, (const void*)(m_data + field.mem_offset), ti_field->GetMemSize());
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
			memcpy((void*)(m_data + field.mem_offset), (const void*)v->m_data, ti_field->GetMemSize());
			return;
		}
	}
	panicf("field[%s] not exists", field_name.c_str());
}
void Variable::Assign(Variable* tmp) {
	assert(m_tid == tmp->GetTypeId());

	//TypeId tmp_tid = m_tid;

	uint8_t* this_m_data = m_data;
	*this				 = *tmp;
	m_data				 = this_m_data;

	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	memcpy(m_data, tmp->m_data, ti->GetMemSize());

	//m_tid	 = tmp_tid;
}
void Variable::InitField(std::map<std::string, Variable*> fields) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	assert(ti->IsTuple() || ti->IsClass());
	assert(ti->GetField().size() == fields.size());

	//m_data = new uint8_t[ti->GetMemSize()];
	for (auto field : ti->GetField()) {
		TypeInfo* ti_field = g_typemgr.GetTypeInfo(field.tid);
		auto	  found	   = fields.find(field.name);
		assert(found != fields.end());
		memcpy((void*)(m_data + field.mem_offset), (const void*)found->second->m_data, ti_field->GetMemSize());
	}
}
void Variable::set_default_value() {
	const TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	if (ti->IsTuple() || ti->IsClass()) {
		m_data = new uint8_t[ti->GetMemSize()];
		for (auto field : ti->GetField()) {
			TypeInfo* ti_field		= g_typemgr.GetTypeInfo(field.tid);
			Variable* default_value = new Variable(field.tid);
			memcpy((void*)(m_data + field.mem_offset), (const void*)default_value->m_data, ti_field->GetMemSize());
		}
	} else if (ti->IsArray()) {
		const TypeInfoArray* ti_array = dynamic_cast<const TypeInfoArray*>(ti);
		if (ti_array->IsStaticSize()) {
			m_data = new uint8_t[ti->GetMemSize()];
		} else {
			uint8_t** value_ptr = new uint8_t*();
			*value_ptr			= new uint8_t[ti->GetMemSize()];
			*(uint64_t*)m_data	= (uint64_t)value_ptr;
		}
	} else if (ti->IsFn()) {
		// panicf("function can not have default value");
		memset(m_data, 0, ti->GetMemSize());
	} else if (ti->IsPointer()) {
		memset(m_data, 0, ti->GetMemSize());
	} else {
		if (is_integer_type(m_tid)) {
			*(uint64_t*)m_data = 0;
		} else {
			switch (m_tid) {
			case TYPE_ID_TYPE:
				*(TypeId*)m_data = TYPE_ID_NONE;
				break;
			case TYPE_ID_FLOAT:
				*(float*)m_data = 0.0;
				break;
			case TYPE_ID_BOOL:
				*(bool*)m_data = false;
				break;
			case TYPE_ID_STR:
				m_data										  = (uint8_t*)new TypeInfoStr::MemStructure();
				((TypeInfoStr::MemStructure*)m_data)->size	  = 0;
				((TypeInfoStr::MemStructure*)m_data)->data	  = new uint8_t[1];
				((TypeInfoStr::MemStructure*)m_data)->data[0] = '\0';
				break;
			default:
				panicf("unknown type[%d:%s]", m_tid, GET_TYPENAME_C(m_tid));
			}
		}
	}
}
void Variable::construct_init(TypeId tid, const void* data) {
	m_tid = tid;

	TypeInfo* ti = g_typemgr.GetTypeInfo(tid);
	m_data		 = (uint8_t*)std::aligned_alloc(ti->GetMemAlignSize(), ti->GetMemSize());

	if (data != nullptr) {
		memcpy(m_data, data, ti->GetMemSize());
	}
}
