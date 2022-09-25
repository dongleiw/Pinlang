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
	// 实现constraint Add
	{
		AstNodeConstraint*				 constraint		= ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId							 constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_STR, TYPE_ID_STR});
		std::map<std::string, Function*> methods;

		TypeId	  tid  = g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{TYPE_ID_STR}, TYPE_ID_STR);
		Function* f	   = new Function(tid, std::vector<ConcreteGParam>(), builtin_fn_add_str);
		methods["add"] = f;

		AddConstraint(constraint_tid, methods);
	}
	// 实现constraint ToString
	{
		AstNodeConstraint*				 constraint		= ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId							 constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		std::map<std::string, Function*> methods;

		TypeId	  tid		= g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{}, TYPE_ID_STR);
		Function* f			= new Function(tid, std::vector<ConcreteGParam>(), builtin_fn_tostring);
		methods["tostring"] = f;

		AddConstraint(constraint_tid, methods);
	}
}
