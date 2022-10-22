#include "type_int64.h"

#include <assert.h>
#include <map>
#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_type.h"
#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"

static Variable* builtin_fn_add(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	int64_t result = thisobj->GetValueInt64() + args.at(0)->GetValueInt64();
	return new Variable(result);
}

static Variable* builtin_fn_sub(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	int64_t result = thisobj->GetValueInt64() - args.at(0)->GetValueInt64();
	return new Variable(result);
}

static Variable* builtin_fn_mul_int(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	int64_t result = thisobj->GetValueInt64() * args.at(0)->GetValueInt64();
	return new Variable(result);
}

static Variable* builtin_fn_lessThan(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	bool v = thisobj->GetValueInt64() < args.at(0)->GetValueInt64();
	return new Variable(v);
}
static Variable* builtin_fn_lessEqual(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	bool v = thisobj->GetValueInt64() <= args.at(0)->GetValueInt64();
	return new Variable(v);
}
static Variable* builtin_fn_notEqual(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	bool v = thisobj->GetValueInt64() != args.at(0)->GetValueInt64();
	return new Variable(v);
}
static Variable* builtin_fn_greaterThan(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	bool v = thisobj->GetValueInt64() > args.at(0)->GetValueInt64();
	return new Variable(v);
}
static Variable* builtin_fn_greaterEqual(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	bool v = thisobj->GetValueInt64() >= args.at(0)->GetValueInt64();
	return new Variable(v);
}

static Variable* builtin_fn_div(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	int64_t result = thisobj->GetValueInt64() / args.at(0)->GetValueInt64();
	return new Variable(result);
}

static Variable* builtin_fn_mod(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT64);
	int64_t result = thisobj->GetValueInt64() % args.at(0)->GetValueInt64();
	return new Variable(result);
}
static Variable* builtin_fn_tostring(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_INT64 && args.size() == 0);
	char buf[16];
	snprintf(buf, sizeof(buf), "%ld", thisobj->GetValueInt64());
	return new Variable(std::string(buf));
}

TypeInfoInt64::TypeInfoInt64() {
	m_name			 = "i64";
	m_typegroup_id	 = TYPE_GROUP_ID_PRIMARY;
	m_mem_size		 = 8;
	m_mem_align_size = 8;
}
void TypeInfoInt64::InitBuiltinMethods(VerifyContext& ctx) {
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
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_tostring));
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
					another_value_type->InitWithIdentifier("i64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("i64");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_add));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_INT64, TYPE_ID_INT64});
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
					another_value_type->InitWithIdentifier("i64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_lessThan));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("lessThan", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("LessThan")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_INT64});
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
					another_value_type->InitWithIdentifier("i64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_lessEqual));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("lessEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("LessEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_INT64});
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
					another_value_type->InitWithIdentifier("i64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_notEqual));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("notEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("NotEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_INT64});
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
					another_value_type->InitWithIdentifier("i64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_greaterThan));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterThan", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("GreaterThan")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_INT64});
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
					another_value_type->InitWithIdentifier("i64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_greaterEqual));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("GreaterEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_INT64});
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
					another_value_type->InitWithIdentifier("i64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("i64");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_sub));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("sub", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Sub")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_INT64, TYPE_ID_INT64});
		AddConstraint(constraint_tid, fns);
	}
	ctx.PopSTack();
}
