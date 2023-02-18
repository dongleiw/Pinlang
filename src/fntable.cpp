#include "fntable.h"
#include "astnode_blockstmt.h"
#include "define.h"
#include "instruction.h"
#include "llvm_ir.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable_table.h"
#include <llvm-12/llvm/IR/DerivedTypes.h>
#include <llvm-12/llvm/IR/Type.h>

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
void FnTable::Compile(LLVMIR& llvm_ir) {
	// 构建fn,
	for (UserDefFnInfo& fn_info : m_userdef_fn_table) {
		TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_info.fn_tid));

		// 如果是main函数, 不增加参数类型和返回值类型信息. 否则没法调用了
		std::string fn_name = fn_info.fnname;
		if (fn_info.fnname == "main[]()i32") {
			fn_name = "main";
		}

		// 构建fn type
		llvm::Type* fn_return_type = nullptr;
		if (tifn->GetReturnTypeId() != TYPE_ID_NONE) {
			TypeInfo* ti_return = g_typemgr.GetTypeInfo(tifn->GetReturnTypeId());
			fn_return_type		= ti_return->GetLLVMIRType(llvm_ir);
		} else {
			fn_return_type = llvm::Type::getVoidTy(IRC);
		}
		std::vector<llvm::Type*> fn_arg_types;
		for (size_t i = tifn->GetParamNum(); i > 0; i--) {
			TypeId	  arg_tid = tifn->GetParamType(i - 1);
			TypeInfo* arg_ti  = g_typemgr.GetTypeInfo(arg_tid);

			fn_arg_types.push_back(arg_ti->GetLLVMIRType(llvm_ir));
		}
		llvm::FunctionType* fn_type = llvm::FunctionType::get(fn_return_type, fn_arg_types, false);

		// 构建fn
		fn_info.llvm_ir_fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, fn_name, IRM);
		assert(fn_info.params_name.size() == fn_info.llvm_ir_fn->arg_size());
		for (size_t i = 0; i < fn_info.llvm_ir_fn->arg_size(); i++) {
			fn_info.llvm_ir_fn->getArg(i)->setName(fn_info.params_name.at(i));
		}

		llvm_ir.AddNamedValue(fn_name, fn_info.llvm_ir_fn);
	}
	for (const UserDefFnInfo& fn_info : m_userdef_fn_table) {
		llvm_ir.SetCurFn(fn_info.llvm_ir_fn);

		llvm_ir.EnterBlock();

		// 声明参数到符号表
		assert(fn_info.params_name.size() == fn_info.llvm_ir_fn->arg_size());
		for (size_t i = 0; i < fn_info.llvm_ir_fn->arg_size(); i++) {
			llvm_ir.AddNamedValue(fn_info.params_name.at(i), fn_info.llvm_ir_fn->getArg(i));
		}
		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(IRC, "entry", fn_info.llvm_ir_fn);
		//llvm_ir.SetExitBlock(llvm::BasicBlock::Create(IRC, "exit", fn_info.llvm_ir_fn));
		IRB.SetInsertPoint(entry_block);

		fn_info.body->Compile(llvm_ir);

		// 处理返回值
		if (fn_info.llvm_ir_fn->getReturnType()->isVoidTy()) {
			IRB.CreateRetVoid();
		}

		llvm_ir.LeaveBlock();

		llvm_ir.SetCurFn(nullptr);
		llvm_ir.SetExitBlock(nullptr);
	}
}
