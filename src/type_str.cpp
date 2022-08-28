#include "type_str.h"

#include <vector>

#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

static Variable* builtin_fn_add_str(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	std::string result = thisobj->GetValueStr() + args.at(0)->GetValueStr();
	return new Variable(result);
}
TypeInfoStr::TypeInfoStr() {
	m_name		   = "str";
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;
}
void TypeInfoStr::InitBuiltinMethods() {
	std::vector<Parameter> params_str;
	params_str.push_back({.arg_tid = TYPE_ID_STR});

	{
		TypeId	  tid = g_typemgr.GetOrAddTypeFn(params_str, TYPE_ID_STR);
		Function* f	  = new Function(tid, builtin_fn_add_str);
		AddMethod("add", f);
	}
}
