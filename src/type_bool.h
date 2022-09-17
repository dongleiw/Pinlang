#pragma once

#include "type.h"

/*
 */
class TypeInfoBool: public TypeInfo {
public:
	TypeInfoBool();
	void InitBuiltinMethods(VerifyContext& ctx) override;

private:
};
