#include "fntable.h"
#include "astnode_blockstmt.h"
#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_class.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable_table.h"
#include <llvm-12/llvm/IR/Argument.h>
#include <llvm-12/llvm/IR/DerivedTypes.h>
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/IR/Type.h>

FnInfo::FnInfo() {
	m_fn_tid	 = TYPE_ID_NONE;
	m_obj_tid	 = TYPE_ID_NONE;
	m_body		 = nullptr;
	m_fn_attr	 = (FnAttr)0;
	m_compile_cb = nullptr;
	m_llvm_ir_fn = nullptr;
	m_is_nested	 = false;
}
FnInfo::FnInfo(std::string fn_name, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<FnParam> params, FnAttr fn_attr, BuiltinFnCompileCallback compile_cb) {
	m_fn_name	 = fn_name;
	m_fn_tid	 = fn_tid;
	m_obj_tid	 = obj_tid;
	m_gparams	 = gparams;
	m_params	 = params;
	m_body		 = nullptr;
	m_fn_attr	 = fn_attr;
	m_compile_cb = compile_cb;
	m_llvm_ir_fn = nullptr;
	m_is_nested	 = false;

	if (fn_attr & FnAttr::FN_ATTR_CONSTRUCTOR) {
		assert(m_obj_tid != TYPE_ID_NONE);
	}
}
FnInfo::FnInfo(std::string fn_name, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<FnParam> params, FnAttr fn_attr, AstNodeBlockStmt* body, bool is_nested) {
	m_fn_name	 = fn_name;
	m_fn_tid	 = fn_tid;
	m_obj_tid	 = obj_tid;
	m_gparams	 = gparams;
	m_params	 = params;
	m_body		 = body;
	m_fn_attr	 = fn_attr;
	m_compile_cb = nullptr;
	m_llvm_ir_fn = nullptr;
	m_is_nested	 = is_nested;
}
bool FnInfo::IsMethod() const {
	return m_obj_tid != TYPE_ID_NONE;
}
bool FnInfo::HasSelfParam() const {
	return m_params.size() > 0 && m_params.at(0).IsSelfParam();
}
llvm::Function* FnInfo::GetLLVMIRFn() const {
	return m_llvm_ir_fn;
}
bool FnInfo::IsConstructor() const {
	return m_fn_attr & FnAttr::FN_ATTR_CONSTRUCTOR;
}
std::string FnInfo::GetFnName() const {
	return m_fn_name;
}
std::string FnInfo::GetFnId() const {
	return m_fn_id;
}
TypeId FnInfo::GetObjTid() const {
	return m_obj_tid;
}
std::vector<ConcreteGParam> FnInfo::GetGParams() const {
	return m_gparams;
}
std::vector<FnParam> FnInfo::GetParams() const {
	return m_params;
}
size_t FnInfo::GetParamNum() const {
	return m_params.size();
}
size_t FnInfo::GetParamNum_no_self() const {
	return m_params.size() - (HasSelfParam() ? 1 : 0);
}
TypeId FnInfo::GetParamType_no_self(size_t idx) const {
	if (HasSelfParam()) {
		idx++;
	}
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_fn_tid));
	return tifn->GetParamType(idx);
}
std::string FnInfo::GetParamName(size_t i) const {
	return m_params.at(i).GetParamName();
}
TypeId FnInfo::GetFnTid() const {
	return m_fn_tid;
}
bool FnInfo::IsNested() const {
	return m_is_nested;
}
AstNodeBlockStmt* FnInfo::GetBody() const {
	return m_body;
}
bool FnInfo::IsBuiltin() const {
	return m_compile_cb != nullptr;
}
BuiltinFnCompileCallback FnInfo::GetCompileCB() const {
	return m_compile_cb;
}
void FnInfo::SetLLVMIRFn(llvm::Function* llvm_ir_fn) {
	m_llvm_ir_fn = llvm_ir_fn;
}
void FnInfo::SetFnId(std::string fn_id) {
	m_fn_id = fn_id;
}
std::string FnTable::AddUserDefineFn(VerifyContext& ctx, FnInfo fn_info) {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_info.GetFnTid()));

	if (fn_info.IsBuiltin()) {
		assert(!fn_info.IsNested());
	}

	std::string fn_id = generate_fnid(fn_info.GetFnName(), fn_info.GetObjTid(), fn_info.GetGParams(), fn_info.GetFnTid(), fn_info.IsNested());
	fn_info.SetFnId(fn_id);

	if (!fn_info.IsBuiltin()) {
		// 构造block
		VariableTable* params_vt = new VariableTable();
		// 将泛参定义到block中
		for (auto iter : fn_info.GetGParams()) {
			params_vt->AddVariable(iter.gparam_name, Variable::CreateTypeVariable(iter.gparam_tid));
		}
		// 将参数定义到block中
		for (size_t i = 0; i < tifn->GetParamNum(); i++) {
			params_vt->AddVariable(fn_info.GetParams().at(i).GetParamName(), new Variable(tifn->GetParamType(i)));
		}

		ctx.PushStack();
		ctx.GetCurStack()->EnterBlock(params_vt);
		fn_info.GetBody()->Verify(ctx, VerifyContextParam().SetExpectReturnTid(tifn->GetReturnTypeId()));
		ctx.PopStack();
	}

	if (m_fn_table.find(fn_id) != m_fn_table.end()) {
		panicf("duplicate fn_id[%s]", fn_id.c_str());
	}
	m_fn_table[fn_id] = fn_info;
	return fn_id;
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
	return found->second.GetFnTid();
}
const FnInfo* FnTable::GetFnInfo(std::string fnid) const {
	auto found = m_fn_table.find(fnid);
	if (found == m_fn_table.end()) {
		panicf("unknown fnid[%s]", fnid.c_str());
	}
	return &(found->second);
}
bool FnTable::IsMethod(std::string fnid) const {
	auto found = m_fn_table.find(fnid);
	if (found == m_fn_table.end()) {
		panicf("unknown fnid[%s]", fnid.c_str());
	}
	return found->second.IsMethod();
}
bool FnTable::IsConstructor(std::string fnid) const {
	auto found = m_fn_table.find(fnid);
	if (found == m_fn_table.end()) {
		panicf("unknown fnid[%s]", fnid.c_str());
	}
	return found->second.IsConstructor();
}
void FnTable::Compile(CompileContext& cctx) {
	compile_builtin_fn(cctx);
	compile_user_define_fn(cctx);
}
void FnTable::compile_user_define_fn(CompileContext& cctx) {
	// 构建fn
	for (auto iter = m_fn_table.begin(); iter != m_fn_table.end(); iter++) {
		FnInfo& fn_info = iter->second;
		if (fn_info.IsBuiltin()) {
			continue;
		}
		TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_info.GetFnTid()));
		assert(fn_info.GetParamNum() == tifn->GetParamNum());

		// 构建fn type
		llvm::Type* fn_return_type = nullptr;
		if (tifn->GetReturnTypeId() != TYPE_ID_NONE) {
			TypeInfo* ti_return = g_typemgr.GetTypeInfo(tifn->GetReturnTypeId());
			fn_return_type		= ti_return->GetLLVMIRType(cctx);
		} else {
			fn_return_type = llvm::Type::getVoidTy(IRC);
		}
		std::vector<llvm::Type*> fn_arg_types;
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
		// 如果是main函数, 不增加参数类型和返回值类型信息. 否则没法调用了
		std::string fn_id = fn_info.GetFnId();
		if (fn_id == "main[]()i32") {
			fn_id = "main";
		}
		fn_info.SetLLVMIRFn(llvm::Function::Create(fn_type, llvm::Function::ExternalLinkage, fn_id, IRM));
		for (size_t i = 0; i < tifn->GetParamNum(); i++) {
			llvm::Argument* arg = fn_info.GetLLVMIRFn()->getArg(i);

			arg->setName(fn_info.GetParamName(i));

			TypeId	  arg_tid = tifn->GetParamType(i);
			TypeInfo* arg_ti  = g_typemgr.GetTypeInfo(arg_tid);

			llvm::Type* ir_type_arg = arg_ti->GetLLVMIRType(cctx);
			if (arg_ti->IsArray()) {
				// 如果函数参数类型为[N]T, 则替换为*[N]T, 并添加byval属性.
				// 这样llvm会将数组clone一份新数据, 然后将新的数组的指针传递给callee
				//llvm::AttrBuilder attr_builder;
				//attr_builder.addByValAttr(ir_type_arg);
				//arg->addAttrs(attr_builder);
			}
		}

		cctx.AddNamedValue(fn_id, fn_info.GetLLVMIRFn());
	}

	// 编译fn
	for (auto iter = m_fn_table.begin(); iter != m_fn_table.end(); iter++) {
		FnInfo& fn_info = iter->second;
		if (fn_info.IsBuiltin()) {
			continue;
		}
		TypeInfoFn*		tifn	   = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(fn_info.GetFnTid()));
		llvm::Function* llvm_ir_fn = fn_info.GetLLVMIRFn();
		cctx.SetCurFn(llvm_ir_fn);
		cctx.SetCurFnIsMethod(fn_info.GetObjTid() != TYPE_ID_NONE);

		cctx.EnterBlock();

		// 声明参数到符号表
		for (size_t i = 0; i < llvm_ir_fn->arg_size(); i++) {
			llvm::Argument* arg = llvm_ir_fn->getArg(i);
			cctx.AddNamedValue(arg->getName().str(), arg);
		}
		llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(IRC, "entry", llvm_ir_fn);
		IRB.SetInsertPoint(entry_block);

		// 对所有参数(除self指针)进行rvalue=>lvalue的处理. 后续再考虑优化
		{
			assert(tifn->GetParamNum() == llvm_ir_fn->arg_size());
			for (size_t i = 0; i < tifn->GetParamNum(); i++) {
				TypeInfo* arg_ti = g_typemgr.GetTypeInfo(tifn->GetParamType(i));
				if (i == 0 && fn_info.HasSelfParam()) {
					if (arg_ti->IsPointer()) {
						// 该函数第一个参数是一个*self
						// 指针self只能作为rvalue使用
					} else {
						// 该函数第一个参数是一个self
						// 禁止
						panicf("self not allowed");
					}
				} else if (arg_ti->IsArray()) {
					// 参数类型为数组, 编译时会转换为指向数组的指针. caller传递指针给callee.
					// callee克隆一份数组作为真正的参数使用 (这样callee内部对数组修改不会影响原数组)
					// 引出数组类型的参数不能直接进行rvalue=>lvalue的转换
					// 而是需要申请一块内存, 将数组load进来, 然后store到新内存里
					llvm::Type*		ir_type_array = arg_ti->GetLLVMIRType(cctx);
					llvm::Argument* arg			  = llvm_ir_fn->getArg(i);
					assert(ir_type_array->getPointerTo() == arg->getType());
					llvm::Value* array		  = IRB.CreateLoad(ir_type_array, arg);
					llvm::Value* cloned_array = IRB.CreateAlloca(ir_type_array);
					IRB.CreateStore(array, cloned_array);
					cctx.ReplaceNamedValue(arg->getName().str(), cloned_array);
				} else {
					llvm::Argument* arg = llvm_ir_fn->getArg(i);
					llvm::Value*	ptr = IRB.CreateAlloca(arg->getType());
					IRB.CreateStore(arg, ptr);
					cctx.ReplaceNamedValue(arg->getName().str(), ptr);
				}
			}
		}

		// 如果当前函数是构造函数, 则先默认构造所有field. 后面才是构造函数中的初始化代码
		// 如果一个field有显式初始化, 默认构造是否应该去掉?
		if (fn_info.IsConstructor()) {
			assert(fn_info.IsMethod() && fn_info.HasSelfParam());
			// 构造所有fields
			TypeInfoClass* ti_class = dynamic_cast<TypeInfoClass*>(g_typemgr.GetTypeInfo(fn_info.GetObjTid()));
			ti_class->ConstructFields(cctx, llvm_ir_fn->getArg(0));
		}
		fn_info.GetBody()->Compile(cctx);

		// 处理返回值
		if (llvm_ir_fn->getReturnType()->isVoidTy()) {
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
		if (fn_info.IsBuiltin()) {
			fn_info.GetCompileCB()(cctx, fn_info);
		}
	}
}
std::string FnTable::generate_fnid(std::string fnname, TypeId obj_tid, std::vector<ConcreteGParam> concrete_generic_params, TypeId fn_tid, bool is_nested) {
	std::string fnid;
	if (obj_tid != TYPE_ID_NONE) {
		fnid = sprintf_to_stdstr("%s::", GET_TYPENAME_C(obj_tid));
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
		auto found = m_nested_fnid_seed.find(fnid);
		if (found == m_nested_fnid_seed.end()) {
			m_nested_fnid_seed[fnid] = 1;
			fnid += " #1";
		} else {
			found->second++;
			fnid += " #" + to_str(found->second);
		}
	}

	return fnid;
}
