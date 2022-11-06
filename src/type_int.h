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
	void InitBuiltinMethods(VerifyContext& ctx) override;

private:
	BuiltinFnExecuteCallback m_builtin_fn_list_tostring[64];
	BuiltinFnExecuteCallback m_builtin_fn_list_add[64];
	BuiltinFnExecuteCallback m_builtin_fn_list_sub[64];
	BuiltinFnExecuteCallback m_builtin_fn_list_equal[64];
	BuiltinFnExecuteCallback m_builtin_fn_list_lessThan[64];
	BuiltinFnExecuteCallback m_builtin_fn_list_lessEqual[64];
	BuiltinFnExecuteCallback m_builtin_fn_list_notEqual[64];
	BuiltinFnExecuteCallback m_builtin_fn_list_greaterThan[64];
	BuiltinFnExecuteCallback m_builtin_fn_list_greaterEqual[64];
};
