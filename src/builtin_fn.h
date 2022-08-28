#pragma once

#include "variable.h"

Variable* builtin_fn_printf_str_int(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);
