#include "builtin_fn.h"
#include "astnode_complex_fndef.h"
#include "astnode_type.h"
#include "define.h"
#include "function.h"
#include "function_obj.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include <cassert>

Variable* builtin_fn_printf_type(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr);
	assert(args.size() == 2 && args.at(0)->GetTypeId() == TYPE_ID_STR && args.at(1)->GetTypeId() == TYPE_ID_TYPE);
	std::string fmt = args.at(0)->GetValueStr();
	TypeId		tid = args.at(1)->GetValueTid();
	printf(fmt.c_str(), GET_TYPENAME_C(tid));
	return nullptr;
}
Variable* builtin_fn_printf_str_float(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr);
	assert(args.size() == 2 && args.at(0)->GetTypeId() == TYPE_ID_STR && args.at(1)->GetTypeId() == TYPE_ID_FLOAT);
	std::string fmt	 = args.at(0)->GetValueStr();
	float		arg1 = args.at(1)->GetValueFloat();
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
void register_all_builtin_fn(VariableTable& vt) {
}
