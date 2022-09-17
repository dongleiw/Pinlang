#include "type_int.h"

#include <assert.h>
#include <map>
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

static Variable* builtin_fn_add_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() + args.at(0)->GetValueInt();
	return new Variable(result);
}

static Variable* builtin_fn_sub_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() - args.at(0)->GetValueInt();
	return new Variable(result);
}

static Variable* builtin_fn_mul_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() * args.at(0)->GetValueInt();
	return new Variable(result);
}

static Variable* builtin_fn_div_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() / args.at(0)->GetValueInt();
	return new Variable(result);
}

static Variable* builtin_fn_mod_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() % args.at(0)->GetValueInt();
	return new Variable(result);
}
static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(args.size() == 0);
	char buf[16];
	snprintf(buf, sizeof(buf), "%d", thisobj->GetValueInt());
	return new Variable(std::string(buf));
}
TypeInfoInt::TypeInfoInt() {
	m_name		   = "int";
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;
}
void TypeInfoInt::InitBuiltinMethods(VerifyContext& ctx) {
	// 实现constraint Add
	{
		AstNodeConstraint*	   constraint	   = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId				   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_INT, TYPE_ID_INT});
		std::map<std::string, Function*> methods;

		TypeId	  tid = g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT);
		Function* f	  = new Function(tid, builtin_fn_add_int);
		methods["add"] = f;

		AddConstraint(constraint_tid, methods);
	}
	// 实现constraint ToString
	{
		AstNodeConstraint*	   constraint	   = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId				   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		std::map<std::string, Function*> methods;

		TypeId	  tid = g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{}, TYPE_ID_STR);
		Function* f	  = new Function(tid, builtin_fn_tostring);
		methods["tostring"] = f;

		AddConstraint(constraint_tid, methods);
	}
}
