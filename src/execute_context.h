#pragma once

#include "variable_table.h"
#include "stack.h"

/*
 * TODO 释放内存
 */
class ExecuteContext{
public:
	ExecuteContext();

	void PushStack();
	void PopSTack();
	Stack* GetCurStack() { return m_top_stack; }

private:
	Stack* m_top_stack;
	VariableTable m_global_vt;
};
