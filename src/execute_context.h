#pragma once

#include "dynamic_loading.h"
#include "stack.h"
#include "variable_table.h"

class Variable;
/*
 * TODO 释放内存
 */
class ExecuteContext {
public:
	ExecuteContext();

	void		   PushStack();
	void		   PopStack();
	Stack*		   GetCurStack() { return m_top_stack; }
	VariableTable* GetGlobalVt() { return &m_global_vt; }

	DynamicLoading& GetDynamicLoading() { return m_dynamic_loading; }

	bool	  IsAssign() const { return m_assign_value != nullptr; }
	Variable* GetAssignValue() { return m_assign_value; }
	void	  SetAssignValue(Variable* v) { m_assign_value = v; }

private:
private:
	Stack*		   m_top_stack;
	VariableTable  m_global_vt;
	DynamicLoading m_dynamic_loading;
	Variable*	   m_assign_value; // 如果是赋值语句, 则为右值. 否则为null
};
