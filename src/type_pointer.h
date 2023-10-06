#pragma once

#include "define.h"
#include "fntable.h"
#include "type.h"

/*
 * `*T` pointer类型, T为任意类型
 */
class TypeInfoPointer : public TypeInfo {
public:
	TypeInfoPointer(TypeId pointee_tid);
	void				InitBuiltinMethods(VerifyContext& ctx) override;
	virtual llvm::Type* GetLLVMIRType(CompileContext& cctx) override;
	virtual void		ConstructDefault(CompileContext& cctx, llvm::Value* obj) override;

	TypeId GetPointeeTid() const { return m_pointee_tid; }

private:
	TypeId m_pointee_tid;
};
