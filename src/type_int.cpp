#include "type_int.h"

#include <assert.h>
#include <map>
#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_type.h"
#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"

#define make_builtin_fn_add(result_type, int_type_id, fnname)                                                                   \
	Variable* builtin_fn_add_##int_type_id(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) { \
		assert(thisobj->GetTypeId() == int_type_id && args.size() == 1 && args.at(0)->GetTypeId() == int_type_id);              \
		result_type result = thisobj->fnname() + args.at(0)->fnname();                                                          \
		return new Variable(result);                                                                                            \
	}
make_builtin_fn_add(int8_t, TYPE_ID_INT8, GetValueInt8);
make_builtin_fn_add(int16_t, TYPE_ID_INT16, GetValueInt16);
make_builtin_fn_add(int32_t, TYPE_ID_INT32, GetValueInt32);
make_builtin_fn_add(int64_t, TYPE_ID_INT64, GetValueInt64);
make_builtin_fn_add(uint8_t, TYPE_ID_UINT8, GetValueUInt8);
make_builtin_fn_add(uint16_t, TYPE_ID_UINT16, GetValueUInt16);
make_builtin_fn_add(uint32_t, TYPE_ID_UINT32, GetValueUInt32);
make_builtin_fn_add(uint64_t, TYPE_ID_UINT64, GetValueUInt64);

#define make_builtin_fn_sub(result_type, int_type_id, fnname)                                                                   \
	Variable* builtin_fn_sub_##int_type_id(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) { \
		assert(thisobj->GetTypeId() == int_type_id && args.size() == 1 && args.at(0)->GetTypeId() == int_type_id);              \
		result_type result = thisobj->fnname() - args.at(0)->fnname();                                                          \
		return new Variable(result);                                                                                            \
	}
make_builtin_fn_sub(int8_t, TYPE_ID_INT8, GetValueInt8);
make_builtin_fn_sub(int16_t, TYPE_ID_INT16, GetValueInt16);
make_builtin_fn_sub(int32_t, TYPE_ID_INT32, GetValueInt32);
make_builtin_fn_sub(int64_t, TYPE_ID_INT64, GetValueInt64);
make_builtin_fn_sub(uint8_t, TYPE_ID_UINT8, GetValueUInt8);
make_builtin_fn_sub(uint16_t, TYPE_ID_UINT16, GetValueUInt16);
make_builtin_fn_sub(uint32_t, TYPE_ID_UINT32, GetValueUInt32);
make_builtin_fn_sub(uint64_t, TYPE_ID_UINT64, GetValueUInt64);

static Variable* builtin_fn_mul_int(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	int32_t result = thisobj->GetValueInt32() * args.at(0)->GetValueInt32();
	return new Variable(result);
}

#define make_builtin_fn_lessThan(int_type_id, fnname)                                                                                \
	Variable* builtin_fn_lessThan_##int_type_id(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) { \
		assert(thisobj->GetTypeId() == int_type_id && args.size() == 1 && args.at(0)->GetTypeId() == int_type_id);                   \
		bool v = thisobj->fnname() < args.at(0)->fnname();                                                                           \
		return new Variable(v);                                                                                                      \
	}
make_builtin_fn_lessThan(TYPE_ID_INT8, GetValueInt8);
make_builtin_fn_lessThan(TYPE_ID_INT16, GetValueInt16);
make_builtin_fn_lessThan(TYPE_ID_INT32, GetValueInt32);
make_builtin_fn_lessThan(TYPE_ID_INT64, GetValueInt64);
make_builtin_fn_lessThan(TYPE_ID_UINT8, GetValueUInt8);
make_builtin_fn_lessThan(TYPE_ID_UINT16, GetValueUInt16);
make_builtin_fn_lessThan(TYPE_ID_UINT32, GetValueUInt32);
make_builtin_fn_lessThan(TYPE_ID_UINT64, GetValueUInt64);

#define make_builtin_fn_lessEqual(int_type_id, fnname)                                                                                \
	Variable* builtin_fn_lessEqual_##int_type_id(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) { \
		assert(thisobj->GetTypeId() == int_type_id && args.size() == 1 && args.at(0)->GetTypeId() == int_type_id);                    \
		bool v = thisobj->fnname() <= args.at(0)->fnname();                                                                           \
		return new Variable(v);                                                                                                       \
	}
make_builtin_fn_lessEqual(TYPE_ID_INT8, GetValueInt8);
make_builtin_fn_lessEqual(TYPE_ID_INT16, GetValueInt16);
make_builtin_fn_lessEqual(TYPE_ID_INT32, GetValueInt32);
make_builtin_fn_lessEqual(TYPE_ID_INT64, GetValueInt64);
make_builtin_fn_lessEqual(TYPE_ID_UINT8, GetValueUInt8);
make_builtin_fn_lessEqual(TYPE_ID_UINT16, GetValueUInt16);
make_builtin_fn_lessEqual(TYPE_ID_UINT32, GetValueUInt32);
make_builtin_fn_lessEqual(TYPE_ID_UINT64, GetValueUInt64);

#define make_builtin_fn_notEqual(int_type_id, fnname)                                                                                \
	Variable* builtin_fn_notEqual_##int_type_id(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) { \
		assert(thisobj->GetTypeId() == int_type_id && args.size() == 1 && args.at(0)->GetTypeId() == int_type_id);                   \
		bool v = thisobj->fnname() != args.at(0)->fnname();                                                                          \
		return new Variable(v);                                                                                                      \
	}
make_builtin_fn_notEqual(TYPE_ID_INT8, GetValueInt8);
make_builtin_fn_notEqual(TYPE_ID_INT16, GetValueInt16);
make_builtin_fn_notEqual(TYPE_ID_INT32, GetValueInt32);
make_builtin_fn_notEqual(TYPE_ID_INT64, GetValueInt64);
make_builtin_fn_notEqual(TYPE_ID_UINT8, GetValueUInt8);
make_builtin_fn_notEqual(TYPE_ID_UINT16, GetValueUInt16);
make_builtin_fn_notEqual(TYPE_ID_UINT32, GetValueUInt32);
make_builtin_fn_notEqual(TYPE_ID_UINT64, GetValueUInt64);

#define make_builtin_fn_greaterThan(int_type_id, fnname)                                                                                \
	Variable* builtin_fn_greaterThan_##int_type_id(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) { \
		assert(thisobj->GetTypeId() == int_type_id && args.size() == 1 && args.at(0)->GetTypeId() == int_type_id);                      \
		bool v = thisobj->fnname() > args.at(0)->fnname();                                                                              \
		return new Variable(v);                                                                                                         \
	}
make_builtin_fn_greaterThan(TYPE_ID_INT8, GetValueInt8);
make_builtin_fn_greaterThan(TYPE_ID_INT16, GetValueInt16);
make_builtin_fn_greaterThan(TYPE_ID_INT32, GetValueInt32);
make_builtin_fn_greaterThan(TYPE_ID_INT64, GetValueInt64);
make_builtin_fn_greaterThan(TYPE_ID_UINT8, GetValueUInt8);
make_builtin_fn_greaterThan(TYPE_ID_UINT16, GetValueUInt16);
make_builtin_fn_greaterThan(TYPE_ID_UINT32, GetValueUInt32);
make_builtin_fn_greaterThan(TYPE_ID_UINT64, GetValueUInt64);

#define make_builtin_fn_greaterEqual(int_type_id, fnname)                                                                                \
	Variable* builtin_fn_greaterEqual_##int_type_id(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) { \
		assert(thisobj->GetTypeId() == int_type_id && args.size() == 1 && args.at(0)->GetTypeId() == int_type_id);                       \
		bool v = thisobj->fnname() >= args.at(0)->fnname();                                                                              \
		return new Variable(v);                                                                                                          \
	}
make_builtin_fn_greaterEqual(TYPE_ID_INT8, GetValueInt8);
make_builtin_fn_greaterEqual(TYPE_ID_INT16, GetValueInt16);
make_builtin_fn_greaterEqual(TYPE_ID_INT32, GetValueInt32);
make_builtin_fn_greaterEqual(TYPE_ID_INT64, GetValueInt64);
make_builtin_fn_greaterEqual(TYPE_ID_UINT8, GetValueUInt8);
make_builtin_fn_greaterEqual(TYPE_ID_UINT16, GetValueUInt16);
make_builtin_fn_greaterEqual(TYPE_ID_UINT32, GetValueUInt32);
make_builtin_fn_greaterEqual(TYPE_ID_UINT64, GetValueUInt64);

#define make_builtin_fn_tostring(int_type_id, fnname)                                                                                \
	Variable* builtin_fn_tostring_##int_type_id(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) { \
		assert(thisobj->GetTypeId() == int_type_id && args.size() == 0);                                                             \
		return new Variable(int_to_str(thisobj->fnname()));                                                                          \
	}
make_builtin_fn_tostring(TYPE_ID_INT8, GetValueInt8);
make_builtin_fn_tostring(TYPE_ID_INT16, GetValueInt16);
make_builtin_fn_tostring(TYPE_ID_INT32, GetValueInt32);
make_builtin_fn_tostring(TYPE_ID_INT64, GetValueInt64);
make_builtin_fn_tostring(TYPE_ID_UINT8, GetValueUInt8);
make_builtin_fn_tostring(TYPE_ID_UINT16, GetValueUInt16);
make_builtin_fn_tostring(TYPE_ID_UINT32, GetValueUInt32);
make_builtin_fn_tostring(TYPE_ID_UINT64, GetValueUInt64);

TypeInfoInt::TypeInfoInt(TypeId tid) {
	switch (tid) {
	case TYPE_ID_INT8:
		m_name			 = "i8";
		m_mem_size		 = 1;
		m_mem_align_size = 1;
		break;
	case TYPE_ID_INT16:
		m_name			 = "i16";
		m_mem_size		 = 2;
		m_mem_align_size = 2;
		break;
	case TYPE_ID_INT32:
		m_name			 = "i32";
		m_mem_size		 = 4;
		m_mem_align_size = 4;
		break;
	case TYPE_ID_INT64:
		m_name			 = "i64";
		m_mem_size		 = 8;
		m_mem_align_size = 8;
		break;
	case TYPE_ID_UINT8:
		m_name			 = "u8";
		m_mem_size		 = 1;
		m_mem_align_size = 1;
		break;
	case TYPE_ID_UINT16:
		m_name			 = "u16";
		m_mem_size		 = 2;
		m_mem_align_size = 2;
		break;
	case TYPE_ID_UINT32:
		m_name			 = "u32";
		m_mem_size		 = 4;
		m_mem_align_size = 4;
		break;
	case TYPE_ID_UINT64:
		m_name			 = "u64";
		m_mem_size		 = 8;
		m_mem_align_size = 8;
		break;
	default:
		panicf("unexpected type[%d:%s]", tid, GET_TYPENAME_C(tid));
		break;
	}
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;

	m_builtin_fn_list_tostring[TYPE_ID_INT8]   = builtin_fn_tostring_TYPE_ID_INT8;
	m_builtin_fn_list_tostring[TYPE_ID_INT16]  = builtin_fn_tostring_TYPE_ID_INT16;
	m_builtin_fn_list_tostring[TYPE_ID_INT32]  = builtin_fn_tostring_TYPE_ID_INT32;
	m_builtin_fn_list_tostring[TYPE_ID_INT64]  = builtin_fn_tostring_TYPE_ID_INT64;
	m_builtin_fn_list_tostring[TYPE_ID_UINT8]  = builtin_fn_tostring_TYPE_ID_UINT8;
	m_builtin_fn_list_tostring[TYPE_ID_UINT16] = builtin_fn_tostring_TYPE_ID_UINT16;
	m_builtin_fn_list_tostring[TYPE_ID_UINT32] = builtin_fn_tostring_TYPE_ID_UINT32;
	m_builtin_fn_list_tostring[TYPE_ID_UINT64] = builtin_fn_tostring_TYPE_ID_UINT64;

	m_builtin_fn_list_add[TYPE_ID_INT8]	  = builtin_fn_add_TYPE_ID_INT8;
	m_builtin_fn_list_add[TYPE_ID_INT16]  = builtin_fn_add_TYPE_ID_INT16;
	m_builtin_fn_list_add[TYPE_ID_INT32]  = builtin_fn_add_TYPE_ID_INT32;
	m_builtin_fn_list_add[TYPE_ID_INT64]  = builtin_fn_add_TYPE_ID_INT64;
	m_builtin_fn_list_add[TYPE_ID_UINT8]  = builtin_fn_add_TYPE_ID_UINT8;
	m_builtin_fn_list_add[TYPE_ID_UINT16] = builtin_fn_add_TYPE_ID_UINT16;
	m_builtin_fn_list_add[TYPE_ID_UINT32] = builtin_fn_add_TYPE_ID_UINT32;
	m_builtin_fn_list_add[TYPE_ID_UINT64] = builtin_fn_add_TYPE_ID_UINT64;

	m_builtin_fn_list_sub[TYPE_ID_INT8]	  = builtin_fn_sub_TYPE_ID_INT8;
	m_builtin_fn_list_sub[TYPE_ID_INT16]  = builtin_fn_sub_TYPE_ID_INT16;
	m_builtin_fn_list_sub[TYPE_ID_INT32]  = builtin_fn_sub_TYPE_ID_INT32;
	m_builtin_fn_list_sub[TYPE_ID_INT64]  = builtin_fn_sub_TYPE_ID_INT64;
	m_builtin_fn_list_sub[TYPE_ID_UINT8]  = builtin_fn_sub_TYPE_ID_UINT8;
	m_builtin_fn_list_sub[TYPE_ID_UINT16] = builtin_fn_sub_TYPE_ID_UINT16;
	m_builtin_fn_list_sub[TYPE_ID_UINT32] = builtin_fn_sub_TYPE_ID_UINT32;
	m_builtin_fn_list_sub[TYPE_ID_UINT64] = builtin_fn_sub_TYPE_ID_UINT64;

	m_builtin_fn_list_lessThan[TYPE_ID_INT8]   = builtin_fn_lessThan_TYPE_ID_INT8;
	m_builtin_fn_list_lessThan[TYPE_ID_INT16]  = builtin_fn_lessThan_TYPE_ID_INT16;
	m_builtin_fn_list_lessThan[TYPE_ID_INT32]  = builtin_fn_lessThan_TYPE_ID_INT32;
	m_builtin_fn_list_lessThan[TYPE_ID_INT64]  = builtin_fn_lessThan_TYPE_ID_INT64;
	m_builtin_fn_list_lessThan[TYPE_ID_UINT8]  = builtin_fn_lessThan_TYPE_ID_UINT8;
	m_builtin_fn_list_lessThan[TYPE_ID_UINT16] = builtin_fn_lessThan_TYPE_ID_UINT16;
	m_builtin_fn_list_lessThan[TYPE_ID_UINT32] = builtin_fn_lessThan_TYPE_ID_UINT32;
	m_builtin_fn_list_lessThan[TYPE_ID_UINT64] = builtin_fn_lessThan_TYPE_ID_UINT64;

	m_builtin_fn_list_lessEqual[TYPE_ID_INT8]	= builtin_fn_lessEqual_TYPE_ID_INT8;
	m_builtin_fn_list_lessEqual[TYPE_ID_INT16]	= builtin_fn_lessEqual_TYPE_ID_INT16;
	m_builtin_fn_list_lessEqual[TYPE_ID_INT32]	= builtin_fn_lessEqual_TYPE_ID_INT32;
	m_builtin_fn_list_lessEqual[TYPE_ID_INT64]	= builtin_fn_lessEqual_TYPE_ID_INT64;
	m_builtin_fn_list_lessEqual[TYPE_ID_UINT8]	= builtin_fn_lessEqual_TYPE_ID_UINT8;
	m_builtin_fn_list_lessEqual[TYPE_ID_UINT16] = builtin_fn_lessEqual_TYPE_ID_UINT16;
	m_builtin_fn_list_lessEqual[TYPE_ID_UINT32] = builtin_fn_lessEqual_TYPE_ID_UINT32;
	m_builtin_fn_list_lessEqual[TYPE_ID_UINT64] = builtin_fn_lessEqual_TYPE_ID_UINT64;

	m_builtin_fn_list_notEqual[TYPE_ID_INT8]   = builtin_fn_notEqual_TYPE_ID_INT8;
	m_builtin_fn_list_notEqual[TYPE_ID_INT16]  = builtin_fn_notEqual_TYPE_ID_INT16;
	m_builtin_fn_list_notEqual[TYPE_ID_INT32]  = builtin_fn_notEqual_TYPE_ID_INT32;
	m_builtin_fn_list_notEqual[TYPE_ID_INT64]  = builtin_fn_notEqual_TYPE_ID_INT64;
	m_builtin_fn_list_notEqual[TYPE_ID_UINT8]  = builtin_fn_notEqual_TYPE_ID_UINT8;
	m_builtin_fn_list_notEqual[TYPE_ID_UINT16] = builtin_fn_notEqual_TYPE_ID_UINT16;
	m_builtin_fn_list_notEqual[TYPE_ID_UINT32] = builtin_fn_notEqual_TYPE_ID_UINT32;
	m_builtin_fn_list_notEqual[TYPE_ID_UINT64] = builtin_fn_notEqual_TYPE_ID_UINT64;

	m_builtin_fn_list_greaterThan[TYPE_ID_INT8]	  = builtin_fn_greaterThan_TYPE_ID_INT8;
	m_builtin_fn_list_greaterThan[TYPE_ID_INT16]  = builtin_fn_greaterThan_TYPE_ID_INT16;
	m_builtin_fn_list_greaterThan[TYPE_ID_INT32]  = builtin_fn_greaterThan_TYPE_ID_INT32;
	m_builtin_fn_list_greaterThan[TYPE_ID_INT64]  = builtin_fn_greaterThan_TYPE_ID_INT64;
	m_builtin_fn_list_greaterThan[TYPE_ID_UINT8]  = builtin_fn_greaterThan_TYPE_ID_UINT8;
	m_builtin_fn_list_greaterThan[TYPE_ID_UINT16] = builtin_fn_greaterThan_TYPE_ID_UINT16;
	m_builtin_fn_list_greaterThan[TYPE_ID_UINT32] = builtin_fn_greaterThan_TYPE_ID_UINT32;
	m_builtin_fn_list_greaterThan[TYPE_ID_UINT64] = builtin_fn_greaterThan_TYPE_ID_UINT64;

	m_builtin_fn_list_greaterEqual[TYPE_ID_INT8]   = builtin_fn_greaterEqual_TYPE_ID_INT8;
	m_builtin_fn_list_greaterEqual[TYPE_ID_INT16]  = builtin_fn_greaterEqual_TYPE_ID_INT16;
	m_builtin_fn_list_greaterEqual[TYPE_ID_INT32]  = builtin_fn_greaterEqual_TYPE_ID_INT32;
	m_builtin_fn_list_greaterEqual[TYPE_ID_INT64]  = builtin_fn_greaterEqual_TYPE_ID_INT64;
	m_builtin_fn_list_greaterEqual[TYPE_ID_UINT8]  = builtin_fn_greaterEqual_TYPE_ID_UINT8;
	m_builtin_fn_list_greaterEqual[TYPE_ID_UINT16] = builtin_fn_greaterEqual_TYPE_ID_UINT16;
	m_builtin_fn_list_greaterEqual[TYPE_ID_UINT32] = builtin_fn_greaterEqual_TYPE_ID_UINT32;
	m_builtin_fn_list_greaterEqual[TYPE_ID_UINT64] = builtin_fn_greaterEqual_TYPE_ID_UINT64;
}
void TypeInfoInt::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	// 手动实现ToString约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				AstNodeType*					return_type = new AstNodeType();
				return_type->InitWithIdentifier("str");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, m_builtin_fn_list_tostring[m_typeid]));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("tostring", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		AddConstraint(constraint_tid, fns);

		GetConstraintMethod(ctx, "ToString", "tostring", std::vector<TypeId>()); // 触发tostring函数的实例化
	}
	// 手动实现Add约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier(m_name);
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, m_builtin_fn_list_add[m_typeid]));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现Less约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, m_builtin_fn_list_lessThan[m_typeid]));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("lessThan", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("LessThan")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现LessEqual约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, m_builtin_fn_list_lessEqual[m_typeid]));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("lessEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("LessEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现NotEqual约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, m_builtin_fn_list_notEqual[m_typeid]));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("notEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("NotEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现GreaterThan约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, m_builtin_fn_list_greaterThan[m_typeid]));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterThan", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("GreaterThan")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现GreaterEqual约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, m_builtin_fn_list_greaterEqual[m_typeid]));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("GreaterEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现Sub约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier(m_name);
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, m_builtin_fn_list_sub[m_typeid]));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("sub", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Sub")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	ctx.PopSTack();
}
