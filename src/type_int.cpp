#include "type_int.h"

#include <assert.h>
#include <map>
#include <vector>

#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_generic_restriction.h"
#include "type_mgr.h"
#include "variable.h"

static Variable* builtin_fn_add_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() + args.at(0)->GetValueInt();
	return new Variable(result);
}
static Variable* builtin_fn_add_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = (float)thisobj->GetValueInt() + args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_sub_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() - args.at(0)->GetValueInt();
	return new Variable(result);
}
static Variable* builtin_fn_sub_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = (float)thisobj->GetValueInt() - args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_mul_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() * args.at(0)->GetValueInt();
	return new Variable(result);
}
static Variable* builtin_fn_mul_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = (float)thisobj->GetValueInt() * args.at(0)->GetValueFloat();
	return new Variable(result);
}

static Variable* builtin_fn_div_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	int result = thisobj->GetValueInt() / args.at(0)->GetValueInt();
	return new Variable(result);
}
static Variable* builtin_fn_div_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	float result = (float)thisobj->GetValueInt() / args.at(0)->GetValueFloat();
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
void TypeInfoInt::InitBuiltinMethods() {
	//AddBuiltinMethod(TYPE_ID_NONE, "add", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_add_int);
	//AddBuiltinMethod(TYPE_ID_NONE, "add", std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT, builtin_fn_add_float);

	//AddBuiltinMethod(TYPE_ID_NONE, "sub", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_sub_int);
	//AddBuiltinMethod(TYPE_ID_NONE, "sub", std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT, builtin_fn_sub_float);

	//AddBuiltinMethod(TYPE_ID_NONE, "mul", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_mul_int);
	//AddBuiltinMethod(TYPE_ID_NONE, "mul", std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT, builtin_fn_mul_float);

	//AddBuiltinMethod(TYPE_ID_NONE, "div", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_div_int);
	//AddBuiltinMethod(TYPE_ID_NONE, "div", std::vector<TypeId>{TYPE_ID_FLOAT}, TYPE_ID_FLOAT, builtin_fn_div_float);

	//AddBuiltinMethod(TYPE_ID_NONE, "mod", std::vector<TypeId>{TYPE_ID_INT}, TYPE_ID_INT, builtin_fn_mod_int);

	///// 实现内置restriction
	// Add
	{
		TypeId						generic_restriction_tid = g_typemgr.GetTypeIdByName("Add");
		TypeInfoGenericRestriction* ti						= dynamic_cast<TypeInfoGenericRestriction*>(g_typemgr.GetTypeInfo(generic_restriction_tid));

		// 根据restriction泛型生成restriction实例
		std::map<std::string, TypeId> generic_params;
		generic_params["AnotherT"] = TYPE_ID_INT;
		TypeId restriction_tid	   = ti->Instantiate(generic_params);

		std::map<std::string, Function*> methods;
		{
			std::vector<Parameter> params{
				Parameter{
					.arg_tid = TYPE_ID_INT,
				},
			};
			methods["add"] = new Function(g_typemgr.GetOrAddTypeFn(params, TYPE_ID_INT), builtin_fn_add_int);
		}
		AddRestriction(restriction_tid, methods);
	}
	// sub
	{
		TypeId						generic_restriction_tid = g_typemgr.GetTypeIdByName("Sub");
		TypeInfoGenericRestriction* ti						= dynamic_cast<TypeInfoGenericRestriction*>(g_typemgr.GetTypeInfo(generic_restriction_tid));

		// 根据restriction泛型生成restriction实例
		std::map<std::string, TypeId> generic_params;
		generic_params["AnotherT"] = TYPE_ID_INT;
		TypeId restriction_tid	   = ti->Instantiate(generic_params);

		std::map<std::string, Function*> methods;
		{
			std::vector<Parameter> params{
				Parameter{
					.arg_tid = TYPE_ID_INT,
				},
			};
			methods["sub"] = new Function(g_typemgr.GetOrAddTypeFn(params, TYPE_ID_INT), builtin_fn_sub_int);
		}
		AddRestriction(restriction_tid, methods);
	}
	// mul
	{
		TypeId						generic_restriction_tid = g_typemgr.GetTypeIdByName("Mul");
		TypeInfoGenericRestriction* ti						= dynamic_cast<TypeInfoGenericRestriction*>(g_typemgr.GetTypeInfo(generic_restriction_tid));

		// 根据restriction泛型生成restriction实例
		std::map<std::string, TypeId> generic_params;
		generic_params["AnotherT"] = TYPE_ID_INT;
		TypeId restriction_tid	   = ti->Instantiate(generic_params);

		std::map<std::string, Function*> methods;
		{
			std::vector<Parameter> params{
				Parameter{
					.arg_tid = TYPE_ID_INT,
				},
			};
			methods["mul"] = new Function(g_typemgr.GetOrAddTypeFn(params, TYPE_ID_INT), builtin_fn_mul_int);
		}
		AddRestriction(restriction_tid, methods);
	}
	// div
	{
		TypeId						generic_restriction_tid = g_typemgr.GetTypeIdByName("Div");
		TypeInfoGenericRestriction* ti						= dynamic_cast<TypeInfoGenericRestriction*>(g_typemgr.GetTypeInfo(generic_restriction_tid));

		// 根据restriction泛型生成restriction实例
		std::map<std::string, TypeId> generic_params;
		generic_params["AnotherT"] = TYPE_ID_INT;
		TypeId restriction_tid	   = ti->Instantiate(generic_params);

		std::map<std::string, Function*> methods;
		{
			std::vector<Parameter> params{
				Parameter{
					.arg_tid = TYPE_ID_INT,
				},
			};
			methods["div"] = new Function(g_typemgr.GetOrAddTypeFn(params, TYPE_ID_INT), builtin_fn_div_int);
		}
		AddRestriction(restriction_tid, methods);
	}
	// mod
	{
		TypeId						generic_restriction_tid = g_typemgr.GetTypeIdByName("Mod");
		TypeInfoGenericRestriction* ti						= dynamic_cast<TypeInfoGenericRestriction*>(g_typemgr.GetTypeInfo(generic_restriction_tid));

		// 根据restriction泛型生成restriction实例
		std::map<std::string, TypeId> generic_params;
		generic_params["AnotherT"] = TYPE_ID_INT;
		TypeId restriction_tid	   = ti->Instantiate(generic_params);

		std::map<std::string, Function*> methods;
		{
			std::vector<Parameter> params{
				Parameter{
					.arg_tid = TYPE_ID_INT,
				},
			};
			methods["mod"] = new Function(g_typemgr.GetOrAddTypeFn(params, TYPE_ID_INT), builtin_fn_mod_int);
		}
		AddRestriction(restriction_tid, methods);
	}
}
