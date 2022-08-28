#pragma once

#include "type.h"

/*
 * 基础的str类型
 */
class TypeInfoStr : public TypeInfo {
public:
	TypeInfoStr();
	void InitBuiltinMethods() override;

private:
};
