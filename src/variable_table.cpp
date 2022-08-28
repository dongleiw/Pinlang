#include "variable_table.h"
#include "builtin_fn.h"
#include "type.h"
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
		//log_debug("var[%s] not exists", name.c_str());
		return nullptr;
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
	log_debug("add var name[%s] type[%d:%s] to vt: value[%s]", name.c_str(), variable->GetTypeId(), GET_TYPENAME_C(variable->GetTypeId()), variable->ToString().c_str());
	m_table[name] = variable;
}
bool VariableTable::HasCandidateFn(std::string name)const{
	return m_candidate_fn_map.find(name)!=m_candidate_fn_map.end();
}
void VariableTable::AddCandidateFn(std::string name, Function* fn){
	auto iter = m_candidate_fn_map.find(name);
	if(iter==m_candidate_fn_map.end()){
		std::vector<Function*> vec;
		vec.push_back(fn);
		m_candidate_fn_map[name] = vec;
	}else{
		iter->second.push_back(fn);
	}
}
void VariableTable::InitAsGlobal(){
	AddVariable("type", Variable::CreateTypeVariable(TYPE_ID_TYPE));
	AddVariable("int", Variable::CreateTypeVariable(TYPE_ID_INT));
	AddVariable("str", Variable::CreateTypeVariable(TYPE_ID_STR));

	//AddVariable("builtin_fn_printf_str_int", Variable::CreateTypeVariable(TYPE_ID_STR));
	register_all_builtin_fn(*this);
}
