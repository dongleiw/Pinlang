#include "variable.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_array.h"
#include "type_mgr.h"
#include <cassert>

Variable::Variable(TypeId tid) {
	m_tid = tid;
}
Variable::Variable(int value) {
	m_tid		= TYPE_ID_INT;
	m_value_int = value;
}
Variable::Variable(float value) {
	m_tid		  = TYPE_ID_FLOAT;
	m_value_float = value;
}
Variable::Variable(bool value) {
	m_tid		 = TYPE_ID_BOOL;
	m_value_bool = value;
}
Variable::Variable(std::string value) {
	m_tid		= TYPE_ID_STR;
	m_value_str = value;
}
Variable::Variable(FunctionObj fnobj) {
	m_tid		  = fnobj.GetFunction()->GetTypeId();
	m_value_fnobj = fnobj;
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
	const TypeInfoArray* tiarray	 = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(array_tid));
	TypeId				 element_tid = tiarray->GetElementType();
	for (auto iter : array) {
		if (iter->GetTypeId() != element_tid) {
			panicf("bug");
		}
	}

	m_tid		  = array_tid;
	m_value_array = array;
}
Variable* Variable::CreateTypeVariable(TypeId tid) {
	Variable* v	   = new Variable(TYPE_ID_TYPE);
	v->m_value_tid = tid;
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
	case TYPE_ID_INT:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", m_value_int);
		s += buf;
		break;
	case TYPE_ID_STR:
		snprintf(buf, sizeof(buf) - 1, "str(%s)", m_value_str.c_str());
		s += buf;
		break;
	case TYPE_ID_FLOAT:
		snprintf(buf, sizeof(buf) - 1, "float(%f)", m_value_float);
		s += buf;
		break;
	//case TYPE_ID_BOOL:
	//	break;
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
int Variable::GetValueInt() const {
	assert(m_tid == TYPE_ID_INT);
	return m_value_int;
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
	return m_value_str;
}
FunctionObj Variable::GetValueFunctionObj() const {
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
const std::vector<Variable*> Variable::GetValueArray() const {
	assert(g_typemgr.GetTypeInfo(m_tid)->IsArray());
	return m_value_array;
}
Variable* Variable::GetMethodValue(MethodIndex method_idx) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	Function* fn = ti->GetMethodByIdx(method_idx);
	return new Variable(FunctionObj(this, fn));
}
