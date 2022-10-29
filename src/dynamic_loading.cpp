#include "dynamic_loading.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"

#include <bits/stdint-intn.h>
#include <cassert>
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>

static Variable* builtin_fn_dl_open(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_STR);

	DynamicLoading& dl					= ctx.GetDynamicLoading();
	std::string		shared_library_path = args.at(0)->GetValueStr();
	int				dynlib_instance_id	= -1;
	std::string		err;

	if (!dl.OpenSharedLibrary(shared_library_path, dynlib_instance_id, err)) {
		panicf("failed to open shared library[%s]: %s", shared_library_path.c_str(), err.c_str());
	}

	return new Variable(dynlib_instance_id);
}

// 调用动态库加载得到的函数. fn()
static Variable* builtin_fn_call_dynlib_fn_args_void_ret_void(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr && fn->GetDynLibFn() != nullptr && args.empty());
	void* dynlib_fn = fn->GetDynLibFn();
	if (!ctx.GetDynamicLoading().FnExist(fn->GetDynLibInstanceId(), dynlib_fn)) {
		panicf("dynlib_fn not exist. dynlib already closed?");
	}
	typedef void (*FnType)(void);
	((FnType)dynlib_fn)();
	return nullptr;
}

// 调用动态库加载得到的函数. fn(i32)
static Variable* builtin_fn_call_dynlib_fn_args_int_ret_void(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr && fn->GetDynLibFn() != nullptr && args.at(0)->GetTypeId() == TYPE_ID_INT32);
	void* dynlib_fn = fn->GetDynLibFn();
	if (!ctx.GetDynamicLoading().FnExist(fn->GetDynLibInstanceId(), dynlib_fn)) {
		panicf("dynlib_fn not exist. dynlib already closed?");
	}
	typedef void (*FnType)(int32_t);
	((FnType)dynlib_fn)(args.at(0)->GetValueInt32());
	return nullptr;
}

// 调用动态库加载得到的函数. fn(i32,i32)
static Variable* builtin_fn_call_dynlib_fn_args_int_int_ret_void(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr && fn->GetDynLibFn() != nullptr && args.at(0)->GetTypeId() == TYPE_ID_INT32 && args.at(1)->GetTypeId() == TYPE_ID_INT32);
	void* dynlib_fn = fn->GetDynLibFn();
	if (!ctx.GetDynamicLoading().FnExist(fn->GetDynLibInstanceId(), dynlib_fn)) {
		panicf("dynlib_fn not exist. dynlib already closed?");
	}
	typedef void (*FnType)(int32_t, int32_t);
	((FnType)dynlib_fn)(args.at(0)->GetValueInt32(), args.at(1)->GetValueInt32());
	return nullptr;
}

// 调用动态库加载得到的函数. fn(const char*, i32) i32
static Variable* builtin_fn_call_dynlib_fn_args_constcharptr_int_ret_int(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr && fn->GetDynLibFn() != nullptr);
	assert(args.at(0)->GetTypeId() == TYPE_ID_STR);
	assert(args.at(1)->GetTypeId() == TYPE_ID_INT32);
	void* dynlib_fn = fn->GetDynLibFn();
	if (!ctx.GetDynamicLoading().FnExist(fn->GetDynLibInstanceId(), dynlib_fn)) {
		panicf("dynlib_fn not exist. dynlib already closed?");
	}
	typedef int32_t (*FnType)(const char*, int32_t);
	int32_t ret = ((FnType)dynlib_fn)(args.at(0)->GetValueStr(), args.at(1)->GetValueInt32());
	return new Variable(ret);
}

// 调用动态库加载得到的函数. fn(const char*, i32, i32) i32
static Variable* builtin_fn_call_dynlib_fn_args_constcharptr_int_int_ret_int(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr && fn->GetDynLibFn() != nullptr && args.size() == 3);
	assert(args.at(0)->GetTypeId() == TYPE_ID_STR);
	assert(args.at(1)->GetTypeId() == TYPE_ID_INT32);
	assert(args.at(2)->GetTypeId() == TYPE_ID_INT32);
	void* dynlib_fn = fn->GetDynLibFn();
	if (!ctx.GetDynamicLoading().FnExist(fn->GetDynLibInstanceId(), dynlib_fn)) {
		panicf("dynlib_fn not exist. dynlib already closed?");
	}
	typedef int32_t (*FnType)(const char*, int32_t, int32_t);
	int32_t ret = ((FnType)dynlib_fn)(args.at(0)->GetValueStr(), args.at(1)->GetValueInt32(), args.at(2)->GetValueInt32());
	return new Variable(ret);
}
// 调用动态库加载得到的函数. fn(i32,str,i64) i64
static Variable* builtin_fn_call_dynlib_fn_args_i32_str_i64_ret_i64(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr && fn->GetDynLibFn() != nullptr && args.size() == 3);
	assert(args.at(0)->GetTypeId() == TYPE_ID_INT32);
	assert(args.at(1)->GetTypeId() == TYPE_ID_STR);
	assert(args.at(2)->GetTypeId() == TYPE_ID_INT64);

	void* dynlib_fn = fn->GetDynLibFn();
	if (!ctx.GetDynamicLoading().FnExist(fn->GetDynLibInstanceId(), dynlib_fn)) {
		panicf("dynlib_fn not exist. dynlib already closed?");
	}
	typedef int64_t (*FnType)(int32_t, const char*, int64_t);
	int64_t ret = ((FnType)dynlib_fn)(args.at(0)->GetValueInt32(), args.at(1)->GetValueStr(), args.at(2)->GetValueInt64());
	return new Variable(ret);
}

static Variable* builtin_fn_dl_getFn(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr && args.size() == 2 && args.at(0)->GetTypeId() == TYPE_ID_INT32 && args.at(1)->GetTypeId() == TYPE_ID_STR);

	DynamicLoading& dl				   = ctx.GetDynamicLoading();
	int32_t			dynlib_instance_id = args.at(0)->GetValueInt32();
	std::string		fn_name			   = args.at(1)->GetValueStr();
	void*			dynlib_fn;
	std::string		err;

	if (!dl.GetFn(dynlib_instance_id, fn_name, &dynlib_fn, err)) {
		panicf("failed to get fn from shared lib-instance-id[%d]: %s", dynlib_instance_id, err.c_str());
	}

	Variable* return_T	 = ctx.GetCurStack()->GetVariable("T");
	TypeId	  return_tid = return_T->GetValueTid();

	// 检查类型是否支持
	TypeInfoFn* ti_fn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(return_tid));
	for (auto tid : ti_fn->GetParmsTid()) {
		if (!dl.SupportType(tid)) {
			panicf("param type[%d:%s] not support to call shared library", tid, GET_TYPENAME_C(tid));
		}
	}
	if (ti_fn->GetReturnTypeId() != TYPE_ID_NONE && !dl.SupportType(ti_fn->GetReturnTypeId())) {
		panicf("return type[%d:%s] not support to call shared library", ti_fn->GetReturnTypeId(), GET_TYPENAME_C(ti_fn->GetReturnTypeId()));
	}

	// 不知道函数参数名, 先随便生成几个
	std::vector<std::string> params_name;
	for (size_t i = 0; i < ti_fn->GetParamNum(); i++) {
		params_name.push_back("arg_" + int_to_str(i));
	}

	BuiltinFnCallback fn_callback = dl.GetBuiltinFnCallback(ti_fn->GetParmsTid(), ti_fn->GetReturnTypeId());
	Function*		  function	  = new Function(return_tid, TYPE_ID_NONE, std::vector<ConcreteGParam>(), params_name, fn_callback, dynlib_instance_id, dynlib_fn);

	return new Variable(FunctionObj(nullptr, function));
}

DynamicLoading::DynamicLoading() {
}
void DynamicLoading::RegisterFn(AstNodeBlockStmt& block_stmt) {
	block_stmt.AddChildStmt(create_astnode_fn_dl_open());
	block_stmt.AddChildStmt(create_astnode_fn_dl_getFn());
}
AstNodeComplexFnDef* DynamicLoading::create_astnode_fn_dl_open() {
	// fn dl_open(path str) int {}
	std::vector<AstNodeComplexFnDef::Implement> implements;

	AstNodeType* param_1_type = new AstNodeType();
	param_1_type->InitWithIdentifier("str");

	AstNodeType* return_type = new AstNodeType();
	return_type->InitWithIdentifier("int");

	std::vector<ParserGenericParam> gparams;
	std::vector<ParserParameter>	params{ParserParameter{
		   .name = "path",
		   .type = param_1_type,
	   }};

	AstNodeComplexFnDef::Implement implement(gparams, params, return_type, nullptr, builtin_fn_dl_open);
	implements.push_back(implement);

	AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("dl_open", implements);

	return astnode_complex_fndef;
}
AstNodeComplexFnDef* DynamicLoading::create_astnode_fn_dl_getFn() {
	// dl_getFn[T Fn](handler int, fn_name str) T
	std::vector<AstNodeComplexFnDef::Implement> implements;

	std::vector<ParserGenericParam> gparams;
	gparams.push_back({ParserGenericParam{
		.type_name		 = "T",
		.constraint_name = "Fn",
	}});

	AstNodeType* param_1_type = new AstNodeType();
	param_1_type->InitWithIdentifier("int");
	AstNodeType* param_2_type = new AstNodeType();
	param_2_type->InitWithIdentifier("str");
	std::vector<ParserParameter> params{
		ParserParameter{
			.name = "handler",
			.type = param_1_type,
		},
		ParserParameter{
			.name = "fn_name",
			.type = param_2_type,
		}};

	AstNodeType* return_type = new AstNodeType();
	return_type->InitWithIdentifier("T");

	implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_dl_getFn));

	AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("dl_getFn", implements);

	return astnode_complex_fndef;
}
bool DynamicLoading::OpenSharedLibrary(std::string path, int& dynlib_instance_id, std::string& err) {
	void* handler = dlopen(path.c_str(), RTLD_NOW);
	if (handler == NULL) {
		err = dlerror();
		log_error("dlopen(%s) failed: %s", path.c_str(), err.c_str());
		return false;
	}

	DynLib dynlib{
		.path				= path,
		.handler			= handler,
		.dynlib_instance_id = int(m_dynlib_instances.size()),
	};

	m_dynlib_instances.push_back(dynlib);

	dynlib_instance_id = dynlib.dynlib_instance_id;

	return true;
}
bool DynamicLoading::GetFn(int dynlib_instance_id, std::string fn_name, void** dynlib_fn, std::string& err) {
	DynLib& dynlib = m_dynlib_instances.at(dynlib_instance_id);
	auto	found  = dynlib.fn_list.find(fn_name);
	if (found != dynlib.fn_list.end()) {
		*dynlib_fn = found->second;
		return true;
	}
	void* f = dlsym(dynlib.handler, fn_name.c_str());
	if (f == nullptr) {
		err = dlerror();
		log_error("failed to get symbol[%s] from dynlib[%s]: %s", fn_name.c_str(), dynlib.path.c_str(), err.c_str());
		return false;
	}
	dynlib.fn_list[fn_name] = f;
	*dynlib_fn				= f;
	return true;
}
bool DynamicLoading::FnExist(int dynlib_instance_id, void* fn) const {
	if (dynlib_instance_id < 0 || m_dynlib_instances.size() <= dynlib_instance_id) {
		return false;
	}
	const DynLib& dynlib = m_dynlib_instances.at(dynlib_instance_id);

	for (auto iter : dynlib.fn_list) {
		if (iter.second == fn) {
			return true;
		}
	}
	return false;
}
bool DynamicLoading::SupportType(TypeId tid) const {
	// 目前只支持这三个类型
	return (tid == TYPE_ID_INT32 || tid == TYPE_ID_FLOAT || tid == TYPE_ID_STR || tid==TYPE_ID_INT64 || tid==TYPE_ID_UINT64);
}
BuiltinFnCallback DynamicLoading::GetBuiltinFnCallback(std::vector<TypeId> params_tid, TypeId return_tid) const {
	/*
	 * 枚举每一种函数签名, 生成对应处理函数.
	 * 先把libc的几个io函数签名搞了.
	 *		int open(const char *pathname, int flags);
	 *		int open(const char *pathname, int flags, mode_t mode);
	 *		ssize_t read(int fd, void *buf, size_t count);
	 *		ssize_t write(int fd, const void *buf, size_t count);
	 *		int close(int fd);
	 * TODO 这简直无穷无尽了.... 有空研究下libffi
	 */
	if (params_tid.size() == 0) {
		if (return_tid == TYPE_ID_NONE) {
			return builtin_fn_call_dynlib_fn_args_void_ret_void; // fn()
		}
	} else if (params_tid.size() == 1) {
		if (params_tid.at(0) == TYPE_ID_INT32 && return_tid == TYPE_ID_NONE) {
			return builtin_fn_call_dynlib_fn_args_int_ret_void; // fn(int)
		}
	} else if (params_tid.size() == 2) {
		if (params_tid.at(0) == TYPE_ID_INT32 && params_tid.at(1) == TYPE_ID_INT32 && return_tid == TYPE_ID_NONE) {
			return builtin_fn_call_dynlib_fn_args_int_int_ret_void; // fn(int,int)
		} else if (params_tid.at(0) == TYPE_ID_STR && params_tid.at(1) == TYPE_ID_INT32 && return_tid == TYPE_ID_INT32) {
			return builtin_fn_call_dynlib_fn_args_constcharptr_int_ret_int; // fn(const char*, int)int
		}
	} else if (params_tid.size() == 3) {
		if (is_vec_typeid_equal(params_tid, std::vector<TypeId>{TYPE_ID_STR, TYPE_ID_INT32, TYPE_ID_INT32}) && return_tid == TYPE_ID_INT32) {
			return builtin_fn_call_dynlib_fn_args_constcharptr_int_int_ret_int; // fn(const char*, int,int)int
		} else if (is_vec_typeid_equal(params_tid, std::vector<TypeId>{TYPE_ID_INT32, TYPE_ID_STR, TYPE_ID_INT64}) && return_tid == TYPE_ID_INT64) {
			// fn(i32, str, i64) i64
			return builtin_fn_call_dynlib_fn_args_i32_str_i64_ret_i64;
		}
	}
	panicf("not implemented yet");
}
