#include "type_float.h"

#include <vector>

#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include "astnode_restriction.h"

static Variable* builtin_fn_add_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() + args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_sub_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() - args.at(0)->GetValueFloat();
	return new Variable(result);
}
static Variable* builtin_fn_sub_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() - args.at(0)->GetValueInt();
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

static Variable* builtin_fn_mod_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() % args.at(0)->GetValueInt();
	return new Variable(result);
}
TypeInfoFloat::TypeInfoFloat() {
	m_name		   = "float";
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;
}
void TypeInfoFloat::InitBuiltinMethods(VerifyContext& ctx) {
	{
		AstNodeRestriction*	   restriction	   = ctx.GetCurStack()->GetVariable("Add")->GetValueRestriction();
		TypeId				   restriction_tid = restriction->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_FLOAT, TYPE_ID_FLOAT});
		std::map<std::string, Function*> methods;

		TypeId	  tid = g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT);
		Function* f	  = new Function(tid, builtin_fn_add_float);
		methods["add"] = f;

		AddRestriction(restriction_tid, methods);
	}
}
