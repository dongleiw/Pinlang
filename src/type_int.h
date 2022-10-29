#pragma once

#include "define.h"
#include "type.h"

/*
 * int类型. i8 i16 i32 i64 u8 u16 u32 u64
 */
class TypeInfoInt : public TypeInfo {
public:
	TypeInfoInt(TypeId tid);
	void InitBuiltinMethods(VerifyContext& ctx) override;

private:
	//static Variable* builtin_fn_tostring_i8(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args);
	//static Variable* builtin_fn_tostring_i16(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args);
	//static Variable* builtin_fn_tostring_i32(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args);
	//static Variable* builtin_fn_tostring_i64(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args);
	//static Variable* builtin_fn_tostring_u8(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args);
	//static Variable* builtin_fn_tostring_u16(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args);
	//static Variable* builtin_fn_tostring_u32(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args);
	//static Variable* builtin_fn_tostring_u64(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args);

	BuiltinFnCallback m_builtin_fn_list_tostring[64];
	BuiltinFnCallback m_builtin_fn_list_add[64];
	BuiltinFnCallback m_builtin_fn_list_sub[64];
	BuiltinFnCallback m_builtin_fn_list_lessThan[64];
	BuiltinFnCallback m_builtin_fn_list_lessEqual[64];
	BuiltinFnCallback m_builtin_fn_list_notEqual[64];
	BuiltinFnCallback m_builtin_fn_list_greaterThan[64];
	BuiltinFnCallback m_builtin_fn_list_greaterEqual[64];
};
