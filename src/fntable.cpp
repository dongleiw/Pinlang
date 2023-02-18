#include "fntable.h"
#include "astnode_blockstmt.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable_table.h"

Variable* FnTable::CallFn(FnAddr addr, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args) {
	switch (addr.fn_kind) {
	case FN_KIND_USERDEF:
		return call_userDef_fn(m_userdef_fn_table.at(addr.idx), ctx, obj, args);
		break;
	case FN_KIND_BUILTIN:
		return call_builtin_fn(m_builtin_fn_table.at(addr.idx), ctx, obj, args);
		break;
	case FN_KIND_DYNAMIC:
		return call_dynamic_fn(m_dynamic_fn_table.at(addr.idx), ctx, obj, args);
		break;
	default:
		panicf("bug");
		break;
	}
}
Variable* FnTable::call_userDef_fn(UserDefFnInfo& fninfo, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args) {
	assert(fninfo.params_name.size() == args.size());
	// 构造block
	VariableTable* vt_args = new VariableTable();
	// 将泛参定义到block中
	for (auto iter : fninfo.gparams) {
		vt_args->AddVariable(iter.gparam_name, Variable::CreateTypeVariable(iter.gparam_tid));
	}
	// 将参数定义到block中
	for (size_t i = 0; i < args.size(); i++) {
		vt_args->AddVariable(fninfo.params_name.at(i), args.at(i));
	}
	if (obj != nullptr) {
		assert(obj->GetTypeId() == fninfo.obj_tid);
		vt_args->AddVariable("this", obj);
	}

	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(vt_args);
	fninfo.body->Execute(ctx);
	Variable* ret_var = ctx.GetCurStack()->GetReturnedValue();
	ctx.PopStack();
	return ret_var;
}
Variable* FnTable::call_builtin_fn(BuiltinFnInfo& fninfo, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args) {
	assert(fninfo.params_name.size() == args.size());
	// 构造block
	VariableTable* vt_args = new VariableTable();
	// 将泛参定义到block中
	for (auto iter : fninfo.gparams) {
		vt_args->AddVariable(iter.gparam_name, Variable::CreateTypeVariable(iter.gparam_tid));
	}
	// 将参数定义到block中
	for (size_t i = 0; i < args.size(); i++) {
		vt_args->AddVariable(fninfo.params_name.at(i), args.at(i));
	}
	if (obj != nullptr) {
		assert(obj->GetTypeId() == fninfo.obj_tid);
		vt_args->AddVariable("this", obj);
	}

	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(vt_args);
	Variable* ret_var = fninfo.execute_cb(fninfo, ctx, obj, args);
	ctx.PopStack();
	return ret_var;
}
Variable* FnTable::call_dynamic_fn(DynamicFnInfo& fninfo, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args) {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fninfo.fn_tid));
	assert(tifn->GetParamNum() == args.size());
	for (size_t i = 0; i < tifn->GetParamNum(); i++) {
		assert(tifn->GetParamType(i) == args.at(i)->GetTypeId());
	}

	ctx.PushStack();
	Variable* ret_var = fninfo.execute_cb(fninfo, ctx, obj, args);
	ctx.PopStack();
	return ret_var;
}
FnAddr FnTable::AddUserDefineFn(VerifyContext& ctx, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, AstNodeBlockStmt* body, std::string fnname) {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));

	// 构造block
	VariableTable* params_vt = new VariableTable();
	// 将泛参定义到block中
	for (auto iter : gparams) {
		params_vt->AddVariable(iter.gparam_name, Variable::CreateTypeVariable(iter.gparam_tid));
	}
	// 将参数定义到block中
	for (size_t i = 0; i < tifn->GetParamNum(); i++) {
		params_vt->AddVariable(params_name.at(i), new Variable(tifn->GetParamType(i)));
	}
	// 如果是方法, 将this定义到block中
	if (obj_tid != TYPE_ID_NONE) {
		params_vt->AddVariable("this", new Variable(obj_tid));
	}

	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(params_vt);
	body->Verify(ctx, VerifyContextParam().SetReturnTid(tifn->GetReturnTypeId()));
	ctx.PopSTack();

	UserDefFnInfo fninfo = UserDefFnInfo{
		.fnname		 = fnname,
		.fn_tid		 = fn_tid,
		.obj_tid	 = obj_tid,
		.gparams	 = gparams,
		.params_name = params_name,
		.body		 = body,
	};
	FnAddr fnaddr;
	fnaddr.fn_kind = FN_KIND_USERDEF;
	fnaddr.idx	   = m_userdef_fn_table.size();
	m_userdef_fn_table.push_back(fninfo);
	return fnaddr;
}
FnAddr FnTable::AddBuiltinFn(VerifyContext& ctx, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, BuiltinFnVerifyCallback verify_cb, BuiltinFnExecuteCallback exeute_cb) {
	BuiltinFnInfo fninfo = BuiltinFnInfo{
		.fn_tid		 = fn_tid,
		.obj_tid	 = obj_tid,
		.gparams	 = gparams,
		.params_name = params_name,
		.verify_cb	 = verify_cb,
		.execute_cb	 = exeute_cb,
	};
	verify_cb(fninfo, ctx);

	FnAddr fnaddr;
	fnaddr.fn_kind = FN_KIND_BUILTIN;
	fnaddr.idx	   = m_builtin_fn_table.size();
	m_builtin_fn_table.push_back(fninfo);
	return fnaddr;
}
FnAddr FnTable::AddDynamicFn(TypeId fn_tid, int dynlib_instance_id, void* dynlib_fn, DynamicFnExecuteCallback cb) {
	DynamicFnInfo fninfo = DynamicFnInfo{
		.fn_tid				= fn_tid,
		.dynlib_instance_id = dynlib_instance_id,
		.dynlib_fn			= dynlib_fn,
		.execute_cb			= cb,
	};

	FnAddr fnaddr;
	fnaddr.fn_kind = FN_KIND_DYNAMIC;
	fnaddr.idx	   = m_dynamic_fn_table.size();
	m_dynamic_fn_table.push_back(fninfo);
	return fnaddr;
}
TypeId FnTable::GetFnTypeId(FnAddr addr) const {
	switch (addr.fn_kind) {
	case FN_KIND_USERDEF:
		return m_userdef_fn_table.at(addr.idx).fn_tid;
		break;
	case FN_KIND_BUILTIN:
		return m_builtin_fn_table.at(addr.idx).fn_tid;
		break;
	case FN_KIND_DYNAMIC:
		return m_dynamic_fn_table.at(addr.idx).fn_tid;
		break;
	default:
		panicf("bug");
		break;
	}
}
TypeId FnTable::GetFnReturnTypeId(FnAddr addr) const {
	TypeId		fn_tid = GetFnTypeId(addr);
	TypeInfoFn* tifn   = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));
	return tifn->GetReturnTypeId();
}
void FnTable::Compile(VM& vm) {
	for (const UserDefFnInfo& fn : m_userdef_fn_table) {
		TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn.fn_tid));

		FnInstructionMaker maker(fn.fnname);
		MemAddr mem_addr;
		fn.body->Compile(vm, maker, mem_addr);
		maker.Finish();
		vm.AddFn(maker);
	}
}
