#include "fntable.h"
#include "astnode_blockstmt.h"
#include "builtin_fn.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable_table.h"
#include <llvm-12/llvm/IR/Argument.h>
#include <llvm-12/llvm/IR/DerivedTypes.h>
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/IR/Type.h>

Variable* FnTable::CallFn(FnAddr addr, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args) {
	switch (addr.fn_kind) {
	case FN_KIND_USERDEF:
		return call_userDef_fn(m_userdef_fn_table.at(addr.idx), ctx, obj, args);
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
FnAddr FnTable::AddBuiltinFn(VerifyContext& ctx, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, BuiltinFnCompileCallback compile_cb, std::string fnid) {
	BuiltinFnInfo fninfo = BuiltinFnInfo{
		.fnid		 = fnid,
		.fn_tid		 = fn_tid,
		.obj_tid	 = obj_tid,
		.gparams	 = gparams,
		.params_name = params_name,
		.compile_cb	 = compile_cb,
	};

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
std::string FnTable::GetFnId(FnAddr addr) const {
	switch (addr.fn_kind) {
	case FN_KIND_USERDEF:
		return m_userdef_fn_table.at(addr.idx).fnname;
		break;
	case FN_KIND_BUILTIN:
		return m_builtin_fn_table.at(addr.idx).fnid;
		break;
	default:
		panicf("bug");
		break;
	}
}
void FnTable::Compile(CompileContext& cctx) {
	compile_builtin_fn(cctx);
	compile_user_define_fn(cctx);
}
void FnTable::compile_user_define_fn(CompileContext& cctx) {
	// 构建fn,
	for (UserDefFnInfo& fn_info : m_userdef_fn_table) {
		TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_info.fn_tid));

		// 如果是main函数, 不增加参数类型和返回值类型信息. 否则没法调用了
		std::string fn_name = fn_info.fnname;
		if (fn_info.fnname == "main[]()5:i32") {
			fn_name = "main";
		}

		// 构建fn type
		llvm::Type* fn_return_type = nullptr;
		if (tifn->GetReturnTypeId() != TYPE_ID_NONE) {
			TypeInfo* ti_return = g_typemgr.GetTypeInfo(tifn->GetReturnTypeId());
			fn_return_type		= ti_return->GetLLVMIRType(cctx);
		} else {
			fn_return_type = llvm::Type::getVoidTy(IRC);
		}
		std::vector<llvm::Type*> fn_arg_types;
		if (fn_info.obj_tid != TYPE_ID_NONE) {
			// 如果是方法, 增加一个隐藏的this指针指向当前obj
			TypeInfo* ti_obj = g_typemgr.GetTypeInfo(fn_info.obj_tid);
			fn_arg_types.push_back(ti_obj->GetLLVMIRType(cctx)->getPointerTo());
		}
		for (size_t i = 0; i < tifn->GetParamNum(); i++) {
			TypeId	  arg_tid = tifn->GetParamType(i);
			TypeInfo* arg_ti  = g_typemgr.GetTypeInfo(arg_tid);

			llvm::Type* ir_type_arg = arg_ti->GetLLVMIRType(cctx);
			if (arg_ti->IsArray()) {
				// 如果函数参数类型为[N]T, 则替换为*[N]T, 并添加byval属性.
				// 这样llvm会将数组clone一份新数据, 然后将新的数组的指针传递给callee
				ir_type_arg = ir_type_arg->getPointerTo();
			}

			fn_arg_types.push_back(ir_type_arg);
		}
		llvm::FunctionType* fn_type = llvm::FunctionType::get(fn_return_type, fn_arg_types, false);

		// 构建fn
		fn_info.llvm_ir_fn = llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, fn_name, IRM);
		if (fn_info.obj_tid != TYPE_ID_NONE) {
			assert(fn_info.params_name.size() + 1 == fn_info.llvm_ir_fn->arg_size());
			fn_info.llvm_ir_fn->getArg(0)->setName("this");
			for (size_t i = 1; i < fn_info.llvm_ir_fn->arg_size(); i++) {
				llvm::Argument* arg = fn_info.llvm_ir_fn->getArg(i);

				arg->setName(fn_info.params_name.at(i - 1));

				TypeId	  arg_tid = tifn->GetParamType(i - 1);
				TypeInfo* arg_ti  = g_typemgr.GetTypeInfo(arg_tid);

				llvm::Type* ir_type_arg = arg_ti->GetLLVMIRType(cctx);
				if (arg_ti->IsArray()) {
					// 如果函数参数类型为[N]T, 则替换为*[N]T, 并添加byval属性.
					// 这样llvm会将数组clone一份新数据, 然后将新的数组的指针传递给callee
					llvm::AttrBuilder attr_builder;
					attr_builder.addByValAttr(ir_type_arg);
					arg->addAttrs(attr_builder);
				}
			}
		} else {
			assert(fn_info.params_name.size() == fn_info.llvm_ir_fn->arg_size());
			for (size_t i = 0; i < fn_info.llvm_ir_fn->arg_size(); i++) {
				llvm::Argument* arg = fn_info.llvm_ir_fn->getArg(i);

				arg->setName(fn_info.params_name.at(i));

				TypeId	  arg_tid = tifn->GetParamType(i);
				TypeInfo* arg_ti  = g_typemgr.GetTypeInfo(arg_tid);

				llvm::Type* ir_type_arg = arg_ti->GetLLVMIRType(cctx);
				if (arg_ti->IsArray()) {
					// 如果函数参数类型为[N]T, 则替换为*[N]T, 并添加byval属性.
					// 这样llvm会将数组clone一份新数据, 然后将新的数组的指针传递给callee
					llvm::AttrBuilder attr_builder;
					attr_builder.addByValAttr(ir_type_arg);
					arg->addAttrs(attr_builder);
				}
			}
		}

		cctx.AddNamedValue(fn_name, fn_info.llvm_ir_fn);
	}
	for (const UserDefFnInfo& fn_info : m_userdef_fn_table) {
		cctx.SetCurFn(fn_info.llvm_ir_fn);
		cctx.SetCurFnIsMethod(fn_info.obj_tid != TYPE_ID_NONE);

		cctx.EnterBlock();

		// 声明参数到符号表
		for (size_t i = 0; i < fn_info.llvm_ir_fn->arg_size(); i++) {
			llvm::Argument* arg = fn_info.llvm_ir_fn->getArg(i);
			cctx.AddNamedValue(arg->getName().str(), arg);
		}
		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(IRC, "entry", fn_info.llvm_ir_fn);
		IRB.SetInsertPoint(entry_block);

		fn_info.body->Compile(cctx);

		// 处理返回值
		if (fn_info.llvm_ir_fn->getReturnType()->isVoidTy()) {
			IRB.CreateRetVoid();
		}

		cctx.LeaveBlock();

		cctx.SetCurFn(nullptr);
	}
}
/*
 * 内置函数以库的方式提供, 在编译代码时链接得到可执行程序
 */
void FnTable::compile_builtin_fn(CompileContext& cctx) {
	// 构建fn
	for (BuiltinFnInfo& fn_info : m_builtin_fn_table) {
		fn_info.compile_cb(cctx, fn_info.fnid);
	}
}
