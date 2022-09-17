#include "function.h"
#include "define.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"

#include "log.h"
VerifyContextResult Function::Verify(VerifyContext& ctx) {
	VerifyContextResult vr;
	if (m_body != nullptr) {
		TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_typeid));

		VariableTable* params_vt = new VariableTable();
		for (size_t i = 0; i < tifn->GetParamNum(); i++) {
			params_vt->AddVariable(m_params_name.at(i), new Variable(tifn->GetParamType(i)));
		}

		ctx.PushStack();
		ctx.GetCurStack()->EnterBlock(params_vt);

		m_body->Verify(ctx, VerifyContextParam().SetReturnTid(tifn->GetReturnTypeId()));

		ctx.PopSTack();
		return vr;
	}
	panicf("not implemented yet");
}
bool Function::VerifyArgsType(std::vector<TypeId> args_type) const {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_typeid));
	return tifn->VerifyArgsType(args_type);
}
TypeId Function::GetReturnTypeId() const {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_typeid));
	return tifn->GetReturnTypeId();
}
Variable* Function::Call(ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args) {
	if (m_builtin_callback != nullptr) {
		ctx.PushStack();
		Variable* result = m_builtin_callback(ctx, obj, args);
		ctx.PopStack();
		return result;
	} else if (m_body != nullptr) {
		// 构造参数block
		VariableTable* vt_args = new VariableTable();
		for (size_t i = 0; i < args.size(); i++) {
			vt_args->AddVariable(m_params_name.at(i), args.at(i));
		}

		ctx.PushStack();
		ctx.GetCurStack()->EnterBlock(vt_args);
		m_body->Execute(ctx);
		Variable* ret_var = ctx.GetCurStack()->GetReturnedValue();
		ctx.PopStack();
		return ret_var;
	} else {
		panicf("bug");
	}
}
