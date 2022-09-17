#pragma once

#include "type.h"

/*
 * 泛型类型
 * 本身无法执行, 仅用于校验和占位
 */
class TypeInfoGenericType : public TypeInfo {
public:
	TypeInfoGenericType(std::string name);

private:
};
