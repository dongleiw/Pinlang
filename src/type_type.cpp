#include "type_type.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "type_mgr.h"
#include "verify_context.h"
#include "astnode_complex_fndef.h"

static Variable* builtin_fn_equal(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_TYPE && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_TYPE);
	return new Variable(thisobj->GetValueTid() == args.at(0)->GetValueTid());
}
static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_TYPE && args.size() == 0);
	return new Variable(GET_TYPENAME(thisobj->GetValueTid()));
}

TypeInfoType::TypeInfoType() {
	m_name = "type";
}
void TypeInfoType::InitBuiltinMethods(VerifyContext& ctx) {
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
	// 手动实现Equal约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier("type");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_equal));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("equal", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Equal")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_TYPE});
		AddConstraint(constraint_tid, fns);
	}
	ctx.PopSTack();
}
