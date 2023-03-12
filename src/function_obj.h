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
	FunctionObj(Variable* obj) {
		m_obj = obj;
	}

private:
	Variable* m_obj; // 所属对象. 普通函数则为null
};
