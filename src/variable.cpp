#include "variable.h"
#include "define.h"
#include "function.h"
#include "function_obj.h"
#include "log.h"
#include "type.h"
#include "type_array.h"
#include "type_mgr.h"
#include "type_tuple.h"
#include "utils.h"
#include <cassert>

Variable::Variable(TypeId tid) {
	m_tid	 = tid;
	m_is_tmp = true;

	set_default_value();
}
Variable::Variable(int32_t value) {
	m_tid		  = TYPE_ID_INT32;
	m_value_int32 = value;
	m_is_tmp	  = true;
}
Variable::Variable(int64_t value) {
	m_tid		  = TYPE_ID_INT64;
	m_value_int64 = value;
	m_is_tmp	  = true;
}
Variable::Variable(float value) {
	m_tid		  = TYPE_ID_FLOAT;
	m_value_float = value;
	m_is_tmp	  = true;
}
Variable::Variable(bool value) {
	m_tid		 = TYPE_ID_BOOL;
	m_value_bool = value;
	m_is_tmp	 = true;
}
Variable::Variable(std::string value) {
	m_tid		 = TYPE_ID_STR;
	m_value_str	 = new std::string();
	*m_value_str = value;
	m_is_tmp	 = true;
}
Variable::Variable(FunctionObj fnobj) {
	m_tid		   = fnobj.GetFunction()->GetTypeId();
	m_value_fnobj  = new FunctionObj();
	*m_value_fnobj = fnobj;
	m_is_tmp	   = true;
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
	const TypeInfoArray* tiarray	 = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(array_tid));
	TypeId				 element_tid = tiarray->GetElementType();
	for (auto iter : array) {
		if (iter->GetTypeId() != element_tid) {
			panicf("bug");
		}
	}

	m_tid		   = array_tid;
	m_value_array  = new std::vector<Variable*>();
	*m_value_array = array;
	m_is_tmp	   = true;
}
Variable* Variable::CreateTypeVariable(TypeId tid) {
	Variable* v	   = new Variable(TYPE_ID_TYPE);
	v->m_value_tid = tid;
	v->m_is_tmp	   = true;
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
Variable* Variable::CallMethod(ExecuteContext& ctx, MethodIndex method_idx, std::vector<Variable*> args) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	Function* f	 = ti->GetMethodByIdx(method_idx);
	return f->Call(ctx, this, args);
}
std::string Variable::ToString() const {
	std::string s;
	char		buf[128];
	switch (m_tid) {
	case TYPE_ID_TYPE:
		snprintf(buf, sizeof(buf) - 1, "type(%d:%s)", m_value_tid, GET_TYPENAME_C(m_value_tid));
		s += buf;
		break;
	case TYPE_ID_INT32:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", m_value_int32);
		s += buf;
		break;
	case TYPE_ID_STR:
		snprintf(buf, sizeof(buf) - 1, "str(%s)", m_value_str->c_str());
		s += buf;
		break;
	case TYPE_ID_FLOAT:
		snprintf(buf, sizeof(buf) - 1, "float(%f)", m_value_float);
		s += buf;
		break;
	case TYPE_ID_BOOL:
		snprintf(buf, sizeof(buf) - 1, "bool(%s)", m_value_bool == true ? "true" : "false");
		s += buf;
		break;
	default:
		snprintf(buf, sizeof(buf) - 1, "unknown");
		s += buf;
		break;
	}
	return s;
}
TypeId Variable::GetValueTid() const {
	assert(m_tid == TYPE_ID_TYPE);
	return m_value_tid;
}
int32_t Variable::GetValueInt32() const {
	assert(m_tid == TYPE_ID_INT32);
	return m_value_int32;
}
int64_t Variable::GetValueInt64() const {
	assert(m_tid == TYPE_ID_INT64);
	return m_value_int64;
}
float Variable::GetValueFloat() const {
	assert(m_tid == TYPE_ID_FLOAT);
	return m_value_float;
}
bool Variable::GetValueBool() const {
	assert(m_tid == TYPE_ID_BOOL);
	return m_value_bool;
}
std::string Variable::GetValueStr() const {
	assert(m_tid == TYPE_ID_STR);
	return *m_value_str;
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
const std::vector<Variable*>& Variable::GetValueArray() const {
	assert(g_typemgr.GetTypeInfo(m_tid)->IsArray());
	return *m_value_array;
}
Variable* Variable::GetMethodValue(MethodIndex method_idx) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	Function* fn = ti->GetMethodByIdx(method_idx);
	Variable* v	 = new Variable(FunctionObj(this, fn));
	v->SetTmp(false);
	return v;
}
Variable* Variable::GetFieldValue(std::string field_name) {
	auto found = m_fields->find(field_name);
	if (found == m_fields->end()) {
		panicf("field[%s] not exists", field_name.c_str());
	} else {
		return found->second;
	}
}
void Variable::Assign(Variable* tmp) {
	assert(!IsTmp());

	TypeId tmp_tid = m_tid;

	*this = *tmp;

	m_tid	 = tmp_tid;
	m_is_tmp = false;
}
void Variable::InitField(std::map<std::string, Variable*> fields) {
	if (m_fields != nullptr) {
		m_fields = new std::map<std::string, Variable*>();
	}
	*m_fields = fields;
}
void Variable::set_default_value() {
	const TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	if (ti->IsTuple() || ti->IsClass()) {
		m_fields = new std::map<std::string, Variable*>();
		for (auto iter : ti->GetField()) {
			Variable* v = new Variable(iter.tid);
			v->SetTmp(false);
			(*m_fields)[iter.name] = v;
		}
	} else if (ti->IsArray()) {
		m_value_array = new std::vector<Variable*>();
	} else if (ti->IsFn()) {
		//panicf("function can not have default value");
		m_value_fnobj = nullptr;
	} else {
		switch (m_tid) {
		case TYPE_ID_TYPE:
			m_value_tid = TYPE_ID_NONE;
			break;
		case TYPE_ID_INT32:
			m_value_int32 = 0;
			break;
		case TYPE_ID_FLOAT:
			m_value_float = 0.0;
			break;
		case TYPE_ID_BOOL:
			m_value_bool = false;
			break;
		case TYPE_ID_STR:
			m_value_str	 = new std::string();
			*m_value_str = "";
			break;
		default:
			panicf("unknown type[%d:%s]", m_tid, GET_TYPENAME_C(m_tid));
		}
	}
}
