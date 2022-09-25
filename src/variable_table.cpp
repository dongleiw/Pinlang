#include "variable_table.h"
#include "astnode_type.h"
#include "builtin_fn.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include <type_traits>
#include <vector>

VariableTable VariableTable::m_global;

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
	log_debug("add var name[%s] type[%d:%s] to vt: value[%s]", name.c_str(), variable->GetTypeId(), GET_TYPENAME_C(variable->GetTypeId()), variable->ToString().c_str());
	m_table[name] = variable;
}
void VariableTable::InitAsGlobal() {
	AddVariable("type", Variable::CreateTypeVariable(TYPE_ID_TYPE));
	AddVariable("int", Variable::CreateTypeVariable(TYPE_ID_INT));
	AddVariable("float", Variable::CreateTypeVariable(TYPE_ID_FLOAT));
	AddVariable("str", Variable::CreateTypeVariable(TYPE_ID_STR));

	//AddVariable("builtin_fn_printf_str_int", Variable::CreateTypeVariable(TYPE_ID_STR));
	register_all_builtin_fn(*this);

	// 添加内置constraint的变量
	{
		//{
		//	std::vector<ParserFnDeclare> rules;
		//	AstNodeType*				 param_type = new AstNodeType();
		//	param_type->InitWithIdentifier("AnotherT");
		//	AstNodeType* return_type = new AstNodeType();
		//	return_type->InitWithIdentifier("ReturnT");
		//	ParserFnDeclare fndec{
		//		.fnname		 = "add",
		//		.param_list	 = std::vector<ParserParameter>{ParserParameter{
		//			 .name = "another",
		//			 .type = param_type,
		//		 }},
		//		.return_type = return_type,
		//	};
		//	AstNodeGenericConstraint* generic_constraint_add = new AstNodeGenericConstraint("Add", std::vector<std::string>{"AnotherT", "ReturnT"}, rules);
		//	AddVariable("Add", new Variable(generic_constraint_add));
		//}
	}
}
