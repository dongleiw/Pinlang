#pragma once

#include <map>

#include "type.h"
#include "variable.h"

/*
 * 变量表(缩写vt)
 * 管理一个block内的变量
 */
class VariableTable {
public:
	VariableTable() {}

	// 如果找不到, panic
	Variable* GetVariable(std::string name);
	// 如果找不到, NULL
	Variable* GetVariableOrNull(std::string name);
	bool	  HasVariable(std::string name);
	void	  AddVariable(std::string name, Variable* variable);

	/*
	 * 增加函数重载
	 */
	void AddCandidateFn(std::string name, Function* fn);
	bool HasCandidateFn(std::string name)const;
	Function* SelectCandidateFnByArgsTypeId(std::string name, std::vector<TypeId> args_tid)const;
	Function* SelectCandidateFnByTypeId(std::string name, TypeId tid)const;

private:
	std::map<std::string, Variable*> m_table;
	std::map<std::string, std::vector<Function*> > m_candidate_fn_map;
};
