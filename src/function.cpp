#include "function.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "verify_context.h"

#include "log.h"
VerifyContextResult Function::Verify(VerifyContext& ctx) {
	VerifyContextResult vr;
	if (m_body == nullptr)
		return vr;
	panicf("not implemented yet");
}
Variable* Function::Call(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	if (m_builtin_callback != nullptr) {
		ctx.PushStack();
		Variable* result = m_builtin_callback(ctx, thisobj, args);
		ctx.PopSTack();
		return result;
	} else if (m_body != nullptr) {
		panicf("not implemented yet");
	} else {
		panicf("bug");
	}
}
bool Function::VerifyArgsType(std::vector<TypeId> args_type) const {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_typeid));
	return tifn->VerifyArgsType(args_type);
}
TypeId Function::GetReturnTypeId() const {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_typeid));
	return tifn->GetReturnTypeId();
}
