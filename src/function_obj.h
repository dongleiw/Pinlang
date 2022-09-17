#pragma once

#include "define.h"
#include "type.h"

class Variable;
/*
 * 函数对象: 包含函数和所属对象. 可以独立于对象直接调用
 */
class FunctionObj {
public:
	FunctionObj() : m_obj(nullptr), m_fn(nullptr) {}
	FunctionObj(Variable* obj, Function* fn) {
		m_obj = obj;
		m_fn  = fn;
	}

	Function* GetFunction() { return m_fn; }
	Variable* Call(ExecuteContext& ctx, std::vector<Variable*> args);

private:
	Variable* m_obj; // 所属对象. 普通函数则为null
	Function* m_fn;
};
