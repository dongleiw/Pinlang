#pragma once

#include "type.h"

/*
 */
class TypeInfoBool: public TypeInfo {
public:
	TypeInfoBool();
	void InitBuiltinMethods(VerifyContext& ctx) override;
	virtual llvm::Type* GetLLVMIRType(LLVMIR& llvm_ir) override;

private:
};
