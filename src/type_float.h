#pragma once

#include "type.h"

/*
 */
class TypeInfoFloat : public TypeInfo {
public:
	TypeInfoFloat();
	void InitBuiltinMethods(VerifyContext& ctx) override;

private:
};
