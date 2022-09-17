#include "function_obj.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable_table.h"
#include "verify_context.h"
#include "function.h"

#include "log.h"

Variable* FunctionObj::Call(ExecuteContext& ctx, std::vector<Variable*> args) {
	return m_fn->Call(ctx, m_obj,args);
}
