#pragma once

#include "type.h"

/*
 * 类型本身也是一个类型. type is value
 * 允许定义类型为(type)的变量, 并拥有一定的运算能力和方法
 *	比如:
 *		var t = int;
 *		var k = float;
 *		t==int
 *		t!=k
 *
 *		t.TypeName()
 *		t.IsInteger()
 *		t.IsFunction()
 *		t.IsClass()
 */
class TypeInfoType : public TypeInfo{
public:
	TypeInfoType();
};
