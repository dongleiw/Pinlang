#include "type_float.h"

#include <vector>

#include "astnode_constraint.h"
#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

#include <cassert>

static Variable* builtin_fn_add_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() + args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_sub_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() - args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_mul_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() * args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_div_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() / args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(args.size() == 0);
	char buf[16];
	snprintf(buf, sizeof(buf), "%f", thisobj->GetValueFloat());
	return new Variable(std::string(buf));
}

TypeInfoFloat::TypeInfoFloat() {
	m_name		   = "float";
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;
}
void TypeInfoFloat::InitBuiltinMethods(VerifyContext& ctx) {
	// 实现constraint Add
	{
		AstNodeConstraint*				 constraint		= ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId							 constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_FLOAT, TYPE_ID_FLOAT});
		std::map<std::string, Function*> methods;

		TypeId	  tid  = g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT);
		Function* f	   = new Function(tid, std::vector<ConcreteGParam>(), builtin_fn_add_float);
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
