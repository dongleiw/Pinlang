#pragma once

#include "type.h"
#include <llvm-12/llvm/IR/DerivedTypes.h>

/*
 */
class TypeInfoClass : public TypeInfo {
public:
	TypeInfoClass(std::string class_name);
	void				InitBuiltinMethods(VerifyContext& ctx) override;
	virtual llvm::Type* GetLLVMIRType(CompileContext& cctx) override;

private:
	llvm::StructType* m_ir_type;
};
