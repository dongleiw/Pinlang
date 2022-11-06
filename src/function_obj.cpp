#include "function_obj.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable_table.h"
#include "verify_context.h"
#include "execute_context.h"

#include "log.h"

FunctionObj::FunctionObj(FunctionObj& another) {
	assert(&another != this);

	m_obj	   = another.m_obj;
	m_fn_addr = another.m_fn_addr;
}
Variable* FunctionObj::Call(ExecuteContext& ctx, std::vector<Variable*> args) {
	return ctx.GetFnTable().CallFn(m_fn_addr, ctx, m_obj, args);
}
