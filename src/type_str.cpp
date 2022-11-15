#include "type_str.h"

#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "execute_context.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include <cassert>

static Variable* builtin_fn_add_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	std::string result = thisobj->GetValueStr();
	result += args.at(0)->GetValueStr();
	return new Variable(result);
}
static void builtin_fn_add_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_STR);
}

static Variable* builtin_fn_tostring(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(args.size() == 0);
	return thisobj;
}
static void builtin_fn_tostring_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_STR);
}

static Variable* builtin_fn_index_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_STR && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT32);
	const char* value = thisobj->GetValueStr();
	int32_t		index = args.at(0)->GetValueInt32();
	if (index < 0 || index >= thisobj->GetValueStrSize()) {
		panicf("wrong array index");
	}
	int32_t ch = value[index];
	return new Variable(ch);
}
static void builtin_fn_index_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_STR);
}

static Variable* builtin_fn_size_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_STR && args.size() == 0);
	return new Variable(thisobj->GetValueStrSize());
}
static void builtin_fn_size_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_STR);
}

TypeInfoStr::TypeInfoStr() {
	m_name			 = "str";
	m_typegroup_id	 = TYPE_GROUP_ID_PRIMARY;
	m_mem_size		 = 16;
	m_mem_align_size = 16;
}
void TypeInfoStr::InitBuiltinMethods(VerifyContext& ctx) {
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
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_tostring_verify, builtin_fn_tostring));
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
					another_value_type->InitWithIdentifier("str");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}

				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("str");

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_add_verify, builtin_fn_add_execute));
			}

			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());

			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_STR, TYPE_ID_STR});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现Index约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* index_type = new AstNodeType();
					index_type->InitWithIdentifier("i32");
					params.push_back({ParserParameter{
						.name = "a",
						.type = index_type,
					}});
				}

				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("int");

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_index_verify, builtin_fn_index_execute));
			}

			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("index", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());

			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Index")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_STR});
		AddConstraint(constraint_tid, fns);
	}
	// 增加内置方法
	{
		std::vector<AstNodeComplexFnDef*> fns;
		// 增加Size()int
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				AstNodeType*					return_type = new AstNodeType();
				return_type->InitWithIdentifier("i32");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_size_verify, builtin_fn_size_execute));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("Size", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}
		AddConstraint(CONSTRAINT_ID_NONE, fns);
	}
	ctx.PopSTack();
}
