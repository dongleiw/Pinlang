#include "variable_table.h"
#include "astnode_type.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include <type_traits>
#include <vector>

Variable* VariableTable::GetVariable(std::string name) {
	auto ret = m_table.find(name);
	if (ret == m_table.end()) {
		panicf("var[%s] not exists", name.c_str());
	} else {
		return ret->second;
	}
}
Variable* VariableTable::GetVariableOrNull(std::string name) {
	auto ret = m_table.find(name);
	if (ret == m_table.end()) {
		//log_debug("var[%s] not exists", name.c_str());
		return nullptr;
	} else {
		return ret->second;
	}
}
TypeId VariableTable::GetVariableType(std::string name) {
	return GetVariable(name)->GetTypeId();
}
bool VariableTable::HasVariable(std::string name) {
	auto ret = m_table.find(name);
	return ret != m_table.end();
}
void VariableTable::AddVariable(std::string name, Variable* variable) {
	if (HasVariable(name)) {
		panicf("var[%s] already exists", name.c_str());
	}
	log_debug("add variable to vt: name[%s] type[%d:%s]", name.c_str(), variable->GetTypeId(), GET_TYPENAME_C(variable->GetTypeId()));
	m_table[name] = variable;
}
void VariableTable::InitAsGlobal() {
	AddVariable("type", Variable::CreateTypeVariable(TYPE_ID_TYPE));

	AddVariable("int", Variable::CreateTypeVariable(TYPE_ID_INT32));
	AddVariable("i8", Variable::CreateTypeVariable(TYPE_ID_INT8));
	AddVariable("i16", Variable::CreateTypeVariable(TYPE_ID_INT16));
	AddVariable("i32", Variable::CreateTypeVariable(TYPE_ID_INT32));
	AddVariable("i64", Variable::CreateTypeVariable(TYPE_ID_INT64));
	AddVariable("u8", Variable::CreateTypeVariable(TYPE_ID_UINT8));
	AddVariable("u16", Variable::CreateTypeVariable(TYPE_ID_UINT16));
	AddVariable("u32", Variable::CreateTypeVariable(TYPE_ID_UINT32));
	AddVariable("u64", Variable::CreateTypeVariable(TYPE_ID_UINT64));

	AddVariable("f32", Variable::CreateTypeVariable(TYPE_ID_FLOAT32));
	AddVariable("f64", Variable::CreateTypeVariable(TYPE_ID_FLOAT64));
	AddVariable("str", Variable::CreateTypeVariable(TYPE_ID_STR));
	AddVariable("bool", Variable::CreateTypeVariable(TYPE_ID_BOOL));
}
void VariableTable::Destroy() {
	for (auto v : m_table) {
		log_debug("destroy variable[%s]", v.first.c_str());
	}
}
