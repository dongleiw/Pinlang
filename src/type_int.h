#pragma once

#include "define.h"
#include "fntable.h"
#include "type.h"

/*
 * int类型. i8 i16 i32 i64 u8 u16 u32 u64
 */
class TypeInfoInt : public TypeInfo {
public:
	TypeInfoInt(TypeId tid);
	void				InitBuiltinMethods(VerifyContext& ctx) override;
	virtual llvm::Type* GetLLVMIRType(CompileContext& cctx) override;
	virtual void		ConstructDefault(CompileContext& cctx, llvm::Value* obj) override;
};
