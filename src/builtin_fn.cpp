#include "builtin_fn.h"
#include "define.h"
#include "variable.h"

#include <cassert>

void BuiltinFn::Verify(VerifyContext& ctx) {
	this->verify(*this, ctx);
}
Variable* BuiltinFn::Call(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	return this->callback(*this, ctx, thisobj, args);
}
