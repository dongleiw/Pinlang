#include "type_bool.h"
#include "variable.h"
#include "verify_context.h"
#include "astnode_constraint.h"
#include "type_mgr.h"
#include "function.h"

#include <cassert>

static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(args.size() == 0);
	if (thisobj->GetValueBool()) {
		return new Variable("true");
	} else {
		return new Variable("false");
	}
}

TypeInfoBool::TypeInfoBool() {
	m_name		   = "bool";
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;
}
void TypeInfoBool::InitBuiltinMethods(VerifyContext& ctx) {
	// 实现constraint ToString
	{
		AstNodeConstraint*				 constraint		= ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId							 constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		std::map<std::string, Function*> methods;

		TypeId	  tid		= g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{}, TYPE_ID_STR);
		Function* f			= new Function(tid, builtin_fn_tostring);
		methods["tostring"] = f;

		AddConstraint(constraint_tid, methods);
	}
}
