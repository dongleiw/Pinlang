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
}
