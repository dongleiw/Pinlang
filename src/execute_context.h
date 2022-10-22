#pragma once

#include "dynamic_loading.h"
#include "stack.h"
#include "variable_table.h"

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

private:

private:
	Stack*		   m_top_stack;
	VariableTable  m_global_vt;
	DynamicLoading m_dynamic_loading;
};
