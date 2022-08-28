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
	AddBuiltinMethod("add", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_add_int);
	AddBuiltinMethod("sub", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_sub_int);
	AddBuiltinMethod("mul", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_mul_int);
	AddBuiltinMethod("div", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_div_int);
	AddBuiltinMethod("mod", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_mod_int);
}
