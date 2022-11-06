#pragma once

#include "define.h"
#include "fntable.h"
#include "type.h"

class Variable;
/*
 * 函数对象: 包含函数和所属对象. 可以独立于对象直接调用
 */
class FunctionObj {
public:
	FunctionObj(FunctionObj& another);
	FunctionObj(Variable* obj, FnAddr fn_addr) {
		m_obj	   = obj;
		m_fn_addr = fn_addr;
	}

	FnAddr	  GetFnAddr() { return m_fn_addr; }
	Variable* Call(ExecuteContext& ctx, std::vector<Variable*> args);

private:
	Variable* m_obj; // 所属对象. 普通函数则为null
	FnAddr	  m_fn_addr;
};
