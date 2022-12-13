#include "type_float.h"

#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "execute_context.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

#include <cassert>

static Variable* builtin_fn_add_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() + args.at(0)->GetValueFloat();
	return new Variable(result);
}
static void builtin_fn_add_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_FLOAT);
}

static Variable* builtin_fn_sub_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() - args.at(0)->GetValueFloat();
	return new Variable(result);
}
static void builtin_fn_sub_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_FLOAT);
}

static Variable* builtin_fn_greaterThan_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_FLOAT && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_FLOAT);
	bool result = thisobj->GetValueFloat() > args.at(0)->GetValueFloat();
	return new Variable(result);
}
static void builtin_fn_greaterThan_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_FLOAT);
}

static Variable* builtin_fn_div_float(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() / args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_tostring_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(args.size() == 0);
	char buf[16];
	snprintf(buf, sizeof(buf), "%f", thisobj->GetValueFloat());
	return new Variable(std::string(buf));
}
static void builtin_fn_tostring_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_FLOAT);
}

TypeInfoFloat::TypeInfoFloat() {
	m_name			 = "float";
	m_typegroup_id	 = TYPE_GROUP_ID_PRIMARY;
	m_mem_size		 = 4;
	m_mem_align_size = 4;
	m_is_value_type	 = true;
}
void TypeInfoFloat::InitBuiltinMethods(VerifyContext& ctx) {
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
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_tostring_verify, builtin_fn_tostring_execute));
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
					another_value_type->InitWithIdentifier("float");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}

				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("float");

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_add_verify, builtin_fn_add_execute));
			}

			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());

			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_FLOAT, TYPE_ID_FLOAT});
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
					another_value_type->InitWithIdentifier("float");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_greaterThan_verify, builtin_fn_greaterThan_execute));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterThan", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("GreaterThan")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_FLOAT});
		AddConstraint(constraint_tid, fns);
	}
	ctx.PopSTack();
}
