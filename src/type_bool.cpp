#include "type_bool.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

#include <cassert>

static Variable* builtin_fn_tostring_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(args.size() == 0);
	if (thisobj->GetValueBool()) {
		return new Variable(std::string("true"));
	} else {
		return new Variable(std::string("false"));
	}
}
static void builtin_fn_tostring_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_BOOL);
}

TypeInfoBool::TypeInfoBool() {
	m_name			 = "bool";
	m_typegroup_id	 = TYPE_GROUP_ID_PRIMARY;
	m_mem_size		 = 1;
	m_mem_align_size = 1;
	m_is_value_type	 = true;
}
void TypeInfoBool::InitBuiltinMethods(VerifyContext& ctx) {
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
	ctx.PopSTack();
}
