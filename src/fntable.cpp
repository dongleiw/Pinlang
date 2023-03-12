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

std::string FnTable::AddUserDefineFn(VerifyContext& ctx, std::string fnname, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, AstNodeBlockStmt* body, bool is_nested) {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));

	std::string fnid = generate_fnid(fnname, obj_tid, gparams, fn_tid, is_nested);

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

	FnInfo fninfo = FnInfo{
		.fnname		 = fnname,
		.fnid		 = fnid,
		.fn_tid		 = fn_tid,
		.obj_tid	 = obj_tid,
		.gparams	 = gparams,
		.params_name = params_name,
		.body		 = body,
		.compile_cb	 = nullptr,
		.llvm_ir_fn	 = nullptr,
	};

	if (m_fn_table.find(fnid) != m_fn_table.end()) {
		panicf("duplicate fnid[%s]", fnid.c_str());
	}
	m_fn_table[fnid] = fninfo;
	return fnid;
}
std::string FnTable::AddBuiltinFn(VerifyContext& ctx, std::string fnname, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, BuiltinFnCompileCallback compile_cb) {

	std::string fnid = generate_fnid(fnname, obj_tid, gparams, fn_tid, false);

	FnInfo fninfo = FnInfo{
		.fnname		 = fnname,
		.fnid		 = fnid,
		.fn_tid		 = fn_tid,
		.obj_tid	 = obj_tid,
		.gparams	 = gparams,
		.params_name = params_name,
		.body		 = nullptr,
		.compile_cb	 = compile_cb,
		.llvm_ir_fn	 = nullptr,
	};

	if (m_fn_table.find(fnid) != m_fn_table.end()) {
		panicf("duplicate fnid[%s]", fnid.c_str());
	}
	m_fn_table[fnid] = fninfo;
	return fnid;
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
TypeId FnTable::GetFnReturnTypeId(std::string fnid) const {
	TypeId		fn_tid = GetFnTypeId(fnid);
	TypeInfoFn* tifn   = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));
	return tifn->GetReturnTypeId();
}
TypeId FnTable::GetFnTypeId(std::string fnid) const {
	auto found = m_fn_table.find(fnid);
	if (found == m_fn_table.end()) {
		panicf("unknown fnid[%s]", fnid.c_str());
	}
	return found->second.fn_tid;
}
void FnTable::Compile(CompileContext& cctx) {
	compile_builtin_fn(cctx);
	compile_user_define_fn(cctx);
}
void FnTable::compile_user_define_fn(CompileContext& cctx) {
	// 构建fn,
	for (auto iter = m_fn_table.begin(); iter != m_fn_table.end(); iter++) {
		FnInfo& fn_info = iter->second;
		assert((fn_info.body != nullptr && fn_info.compile_cb == nullptr) || (fn_info.body == nullptr && fn_info.compile_cb != nullptr));
		if (fn_info.body == nullptr) {
			continue;
		}
		TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_info.fn_tid));

		// 如果是main函数, 不增加参数类型和返回值类型信息. 否则没法调用了
		std::string fn_name = fn_info.fnid;
		if (fn_info.fnid == "main[]()i32") {
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
	for (auto iter = m_fn_table.begin(); iter != m_fn_table.end(); iter++) {
		FnInfo& fn_info = iter->second;
		assert((fn_info.body != nullptr && fn_info.compile_cb == nullptr) || (fn_info.body == nullptr && fn_info.compile_cb != nullptr));
		if (fn_info.body == nullptr) {
			continue;
		}
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
	for (auto iter = m_fn_table.begin(); iter != m_fn_table.end(); iter++) {
		FnInfo& fn_info = iter->second;
		assert((fn_info.body != nullptr && fn_info.compile_cb == nullptr) || (fn_info.body == nullptr && fn_info.compile_cb != nullptr));
		if (fn_info.body != nullptr) {
			continue;
		}
		fn_info.compile_cb(cctx, fn_info.fnid);
	}
}
std::string FnTable::generate_fnid(std::string fnname, TypeId obj_tid, std::vector<ConcreteGParam> concrete_generic_params, TypeId fn_tid, bool is_nested) {
	std::string fnid;
	if (obj_tid != TYPE_ID_NONE) {
		fnid = sprintf_to_stdstr("%d:%s::", obj_tid, GET_TYPENAME_C(obj_tid));
	}
	fnid += fnname + "[";
	for (size_t i = 0; i < concrete_generic_params.size(); i++) {
		fnid = fnid + sprintf_to_stdstr("%s", GET_TYPENAME_C(concrete_generic_params.at(i).gparam_tid));
		if (i + 1 != concrete_generic_params.size()) {
			fnid += ",";
		}
	}
	fnid += "](";

	// 参数
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_tid));
	for (size_t i = 0; i < tifn->GetParamNum(); i++) {
		fnid = fnid + sprintf_to_stdstr("%s", GET_TYPENAME_C(tifn->GetParamType(i)));
		if (i + 1 != tifn->GetParamNum()) {
			fnid += ",";
		}
	}

	fnid += ")";
	if (tifn->GetReturnTypeId() != TYPE_ID_NONE) {
		fnid += sprintf_to_stdstr("%s", GET_TYPENAME_C(tifn->GetReturnTypeId()));
	}

	if (is_nested) {
		auto found = m_nested_fnid_seed.find(fnname);
		if (found == m_nested_fnid_seed.end()) {
			fnid += " #1";
			m_nested_fnid_seed[fnname] = 1;
		} else {
			fnid += " #" + to_str(found->second);
			found->second++;
		}
	}

	return fnid;
}
