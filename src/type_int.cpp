#include "type_int.h"

#include <vector>

#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

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
TypeInfoInt::TypeInfoInt() {
	m_name		   = "int";
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;
}
void TypeInfoInt::InitBuiltinMethods() {
	std::vector<Parameter> params_int;
	params_int.push_back({.arg_tid = TYPE_ID_INT});

	{
		TypeId	  tid = g_typemgr.GetOrAddTypeFn(params_int, TYPE_ID_INT);
		Function* f	  = new Function(tid, builtin_fn_add_int);
		AddMethod("add", f);
	}
	{
		TypeId	  tid = g_typemgr.GetOrAddTypeFn(params_int, TYPE_ID_INT);
		Function* f	  = new Function(tid, builtin_fn_sub_int);
		AddMethod("sub", f);
	}
	{
		TypeId	  tid = g_typemgr.GetOrAddTypeFn(params_int, TYPE_ID_INT);
		Function* f	  = new Function(tid, builtin_fn_mul_int);
		AddMethod("mul", f);
	}
	{
		TypeId	  tid = g_typemgr.GetOrAddTypeFn(params_int, TYPE_ID_INT);
		Function* f	  = new Function(tid, builtin_fn_div_int);
		AddMethod("div", f);
	}
	{
		TypeId	  tid = g_typemgr.GetOrAddTypeFn(params_int, TYPE_ID_INT);
		Function* f	  = new Function(tid, builtin_fn_mod_int);
		AddMethod("mod", f);
	}
}
