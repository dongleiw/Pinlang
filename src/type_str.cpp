#include "type_str.h"

#include <vector>

#include "astnode_constraint.h"
#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "astnode_complex_fndef.h"

#include <cassert>

static Variable* builtin_fn_add_str(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	std::string result = thisobj->GetValueStr() + args.at(0)->GetValueStr();
	return new Variable(result);
}

static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(args.size() == 0);
	return thisobj;
}

TypeInfoStr::TypeInfoStr() {
	m_name		   = "str";
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;
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
				AstNodeType* return_type = new AstNodeType();
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

		GetConcreteMethod(ctx, "tostring", std::vector<TypeId>(), TYPE_ID_STR);
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

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_add_str));
			}

			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());

			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_STR, TYPE_ID_STR});
		AddConstraint(constraint_tid, fns);
	}
	ctx.PopSTack();
}
