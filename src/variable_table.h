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
	// 根据名字查找变量的类型, 找不到panic
	TypeId GetVariableType(std::string name);
	bool   HasVariable(std::string name);
	void   AddVariable(std::string name, Variable* variable);
	void   Destroy();

	void InitAsGlobal();

private:
	std::map<std::string, Variable*> m_table;
};
