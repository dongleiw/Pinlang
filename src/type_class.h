#pragma once

#include "type.h"

/*
 */
class TypeInfoClass : public TypeInfo {
public:
	TypeInfoClass(std::string class_name);
	void InitBuiltinMethods(VerifyContext& ctx) override;

};
