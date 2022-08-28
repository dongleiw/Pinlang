#include "builtin_fn.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include <assert.h>

Variable* builtin_fn_printf_str(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr);
	assert(args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_STR);
	std::string fmt = args.at(0)->GetValueStr();
	printf(fmt.c_str());
	return nullptr;
}
Variable* builtin_fn_printf_str_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr);
	assert(args.size() == 2 && args.at(0)->GetTypeId() == TYPE_ID_STR && args.at(1)->GetTypeId() == TYPE_ID_INT);
	std::string fmt	 = args.at(0)->GetValueStr();
	int			arg1 = args.at(1)->GetValueInt();
	printf(fmt.c_str(), arg1);
	return nullptr;
}
Variable* builtin_fn_printf_str_str(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr);
	assert(args.size() == 2 && args.at(0)->GetTypeId() == TYPE_ID_STR && args.at(1)->GetTypeId() == TYPE_ID_STR);
	std::string fmt	 = args.at(0)->GetValueStr();
	std::string arg1 = args.at(1)->GetValueStr();
	printf(fmt.c_str(), arg1.c_str());
	return nullptr;
}
void register_builtin_fn(VariableTable& vt, std::string fnname, std::vector<TypeId> params_tid, TypeId ret_tid, BuiltinFnCallback cb) {
	std::string			   uniq_fnname = fnname + "_" + TypeInfoFn::GetUniqFnNameSuffix(params_tid);
	std::vector<Parameter> params;
	for (auto iter : params_tid) {
		params.push_back({.arg_tid = iter});
	}
	TypeId	  tid = g_typemgr.GetOrAddTypeFn(params, ret_tid);
	Function* f	  = new Function(tid, cb);
	vt.AddVariable(uniq_fnname, new Variable(f));
	vt.AddCandidateFn(fnname, f);
}
void register_all_builtin_fn(VariableTable& vt) {
	register_builtin_fn(vt, "printf", std::vector<TypeId>{TYPE_ID_STR, TYPE_ID_INT}, TYPE_ID_NONE, builtin_fn_printf_str_int);
	register_builtin_fn(vt, "printf", std::vector<TypeId>{TYPE_ID_STR, TYPE_ID_STR}, TYPE_ID_NONE, builtin_fn_printf_str_str);
	register_builtin_fn(vt, "printf", std::vector<TypeId>{TYPE_ID_STR}, TYPE_ID_NONE, builtin_fn_printf_str);
}
