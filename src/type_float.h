#pragma once

#include "type.h"

/*
 */
class TypeInfoFloat : public TypeInfo {
public:
	TypeInfoFloat();
	void InitBuiltinMethods() override;

private:
};
