#pragma once

#include "fntable.h"
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

	bool	  IsAssign() const { return m_assign_value != nullptr; }
	Variable* GetAssignValue() { return m_assign_value; }
	void	  SetAssignValue(Variable* v) { m_assign_value = v; }
	void	  SetFnTable(FnTable& fn_table) { m_fn_table = fn_table; }
	FnTable&  GetFnTable() { return m_fn_table; }

private:
private:
	Stack*		  m_top_stack;
	VariableTable m_global_vt;
	Variable*	  m_assign_value; // 如果是赋值语句, 则为右值. 否则为null
	FnTable		  m_fn_table;	  // verify节点得到的函数表
};
