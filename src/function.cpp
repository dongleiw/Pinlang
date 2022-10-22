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

		// 构造block
		VariableTable* params_vt = new VariableTable();
		// 将泛参定义到block中
		for (auto iter : m_gparams) {
			params_vt->AddVariable(iter.gparam_name, Variable::CreateTypeVariable(iter.gparam_tid));
		}
		// 将参数定义到block中
		for (size_t i = 0; i < tifn->GetParamNum(); i++) {
			params_vt->AddVariable(m_params_name.at(i), new Variable(tifn->GetParamType(i)));
		}
		// 如果是方法, 将this定义到block中
		if (m_obj_tid != TYPE_ID_NONE) {
			params_vt->AddVariable("this", new Variable(m_obj_tid));
		}

		ctx.PushStack();
		ctx.GetCurStack()->EnterBlock(params_vt);

		m_body->Verify(ctx, VerifyContextParam().SetReturnTid(tifn->GetReturnTypeId()));

		ctx.PopSTack();
		return vr;
	} else {
		return vr;
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
Variable* Function::Call(ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args) {
	if (m_builtin_callback != nullptr) {
		// 构造block
		VariableTable* vt_args = new VariableTable();
		// 将泛参定义到block中
		for (auto iter : m_gparams) {
			vt_args->AddVariable(iter.gparam_name, Variable::CreateTypeVariable(iter.gparam_tid));
		}
		// 将参数定义到block中
		for (size_t i = 0; i < args.size(); i++) {
			vt_args->AddVariable(m_params_name.at(i), args.at(i));
		}
		if (obj != nullptr) {
			assert(obj->GetTypeId() == m_obj_tid);
			vt_args->AddVariable("this", obj);
		}

		ctx.PushStack();
		ctx.GetCurStack()->EnterBlock(vt_args);
		Variable* ret_var = m_builtin_callback(ctx, this, obj, args);
		ctx.PopStack();
		return ret_var;
	} else if (m_body != nullptr) {
		// 构造block
		VariableTable* vt_args = new VariableTable();
		// 将泛参定义到block中
		for (auto iter : m_gparams) {
			vt_args->AddVariable(iter.gparam_name, Variable::CreateTypeVariable(iter.gparam_tid));
		}
		// 将参数定义到block中
		for (size_t i = 0; i < args.size(); i++) {
			vt_args->AddVariable(m_params_name.at(i), args.at(i));
		}
		if (obj != nullptr) {
			assert(obj->GetTypeId() == m_obj_tid);
			vt_args->AddVariable("this", obj);
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
