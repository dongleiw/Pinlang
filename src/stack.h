#pragma once

#include <list>
#include <utility>

#include "variable.h"
#include "variable_table.h"

/*
 * 执行过程中的一个栈. 每次函数调用入栈, 函数结束出栈
 */
class Stack {
public:
	Stack(Stack* next);

	void   EnterBlock(VariableTable* vt);
	void   LeaveBlock();
	bool   IsBottomStck() const { return m_next == NULL; }
	Stack* GetNextStack() { return m_next; }

	VariableTable* GetCurVariableTable();
	bool		   IsVariableExist(std::string name);
	// 如果找不到, 返回NULL
	Variable* GetVariableOrNull(std::string name);
	// 根据name查找变量的类型id. 返回类型id. 找不到panic
	TypeId GetVariableType(std::string name);
	// 如果找不到, panic
	Variable* GetVariable(std::string name);

	Variable* GetReturnedValue() const { return m_returned_value; }
	void	  SetReturnedValue(Variable* v) { m_returned_value = v; }

	VariableTable* GetVariableTableByFnName(std::string fnname);
	VariableTable* GetVariableTableByVarName(std::string varname);
private:
	Stack*					  m_next;	 // 下一层栈
	std::list<VariableTable*> m_vt_list; // 变量表列表. 后面vt是前面vt的孩子
	Variable*				  m_returned_value;
};
