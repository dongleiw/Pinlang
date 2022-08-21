#include "stack.h"
#include "log.h"
#include "variable.h"
#include "variable_table.h"

#include <assert.h>

Stack::Stack(Stack* next) : m_next(next) {
	m_vt_list.push_back(new VariableTable());
}

void Stack::EnterBlock(VariableTable* vt) {
	assert(vt != nullptr);
	m_vt_list.push_back(vt);
}
void Stack::LeaveBlock() {
	assert(m_vt_list.empty() == false);
	m_vt_list.pop_back();
}
VariableTable* Stack::GetCurVariableTable() {
	return *m_vt_list.rbegin();
}
bool Stack::IsVariableExist(std::string name) {
	for (auto iter = m_vt_list.rbegin(); iter != m_vt_list.rend(); iter++) {
		VariableTable* vt = *iter;
		if (vt->HasVariable(name)) {
			return true;
		}
	}
	return false;
}
Variable* Stack::GetVariableOrNull(std::string name) {
	for (auto iter = m_vt_list.rbegin(); iter != m_vt_list.rend(); iter++) {
		VariableTable* vt = *iter;
		Variable*	   v  = vt->GetVariableOrNull(name);
		if (v != nullptr)
			return v;
	}
	return nullptr;
}
Variable* Stack::GetVariable(std::string name) {
	Variable* v = GetVariableOrNull(name);
	if (v == nullptr) {
		panicf("var[%s] not exist", name.c_str());
	}
	return v;
}
VariableTable* Stack::GetVariableTableByFnName(std::string fnname) {
	for (auto iter = m_vt_list.rbegin(); iter != m_vt_list.rend(); iter++) {
		VariableTable* vt = *iter;
		if (vt->HasCandidateFn(fnname))
			return vt;
	}
	return nullptr;
}
