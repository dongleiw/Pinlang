#include "builtin_fn.h"
#include "type.h"

#include <assert.h>

Variable* builtin_fn_printf_str_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj==nullptr);
	assert(args.size()==2 && args.at(0)->GetTypeId()==TYPE_ID_STR && args.at(1)->GetTypeId()==TYPE_ID_INT);
	int result = thisobj->GetValueInt() + args.at(0)->GetValueInt();
	return new Variable(result);
}
