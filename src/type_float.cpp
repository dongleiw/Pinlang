#include "type_float.h"

#include <vector>

#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

static Variable* builtin_fn_add_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() + args.at(0)->GetValueFloat();
	return new Variable(result);
}
static Variable* builtin_fn_add_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = thisobj->GetValueFloat() + (float)args.at(0)->GetValueInt();
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
void TypeInfoFloat::InitBuiltinMethods() {
	AddBuiltinMethod("add", std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT, builtin_fn_add_float);
	AddBuiltinMethod("add", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_FLOAT, builtin_fn_add_int);

	AddBuiltinMethod("sub", std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT, builtin_fn_sub_float);
	AddBuiltinMethod("sub", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_FLOAT, builtin_fn_sub_int);

	AddBuiltinMethod("mul", std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT, builtin_fn_mul_float);

	AddBuiltinMethod("div", std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT, builtin_fn_div_float);

	AddBuiltinMethod("mod", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_mod_int);
}
