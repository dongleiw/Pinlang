#include "variable_table.h"
#include "type_mgr.h"
#include "log.h"

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
		log_debug("var[%s] not exists", name.c_str());
		return NULL;
	} else {
		return ret->second;
	}
}
bool VariableTable::HasVariable(std::string name) {
	auto ret = m_table.find(name);
	return ret != m_table.end();
}
void VariableTable::AddVariable(std::string name, Variable* variable) {
	if (HasVariable(name)) {
		panicf("var[%s] already exists", name.c_str());
	}
	log_debug("add var name[%s] type[%d:%s] to vt", name.c_str(), variable->GetTypeId(), GET_TYPENAME_C(variable->GetTypeId()));
	m_table[name] = variable;
}
