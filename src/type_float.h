#pragma once

#include "define.h"
#include "type.h"

/*
 * f32 f64
 */
class TypeInfoFloat : public TypeInfo {
public:
	TypeInfoFloat(TypeId tid);
	void				InitBuiltinMethods(VerifyContext& ctx) override;
	virtual llvm::Type* GetLLVMIRType(CompileContext& cctx) override;

private:
};
