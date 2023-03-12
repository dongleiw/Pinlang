#pragma once

#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"

#include <llvm-12/llvm/IR/Function.h>
#include <vector>

class AstNodeBlockStmt;
class VerifyContext;

enum FnKind {
	FN_KIND_USERDEF,
	FN_KIND_BUILTIN,
	FN_KIND_DYNAMIC,
};
struct FnAddr {
	FnKind fn_kind;
	int	   idx;

	FnAddr() : idx(-1) {
	}
};

struct BuiltinFnInfo;
struct DynamicFnInfo;

// 指向内置函数的verify函数的指针
typedef void (*BuiltinFnVerifyCallback)(BuiltinFnInfo& builtin_fn, VerifyContext& ctx);

// 指向内置函数的执行函数的指针
typedef Variable* (*BuiltinFnExecuteCallback)(BuiltinFnInfo& builtin_fn, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);

// 指向内置函数的执行函数的指针
typedef Variable* (*DynamicFnExecuteCallback)(DynamicFnInfo& dynamic_fn, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);

// 用户定义函数
struct UserDefFnInfo {
	std::string					fnname;
	TypeId						fn_tid;
	TypeId						obj_tid;
	std::vector<ConcreteGParam> gparams;
	std::vector<std::string>	params_name;
	AstNodeBlockStmt*			body;
	llvm::Function*				llvm_ir_fn;
};
// 编译器内置函数
struct BuiltinFnInfo {
	std::string					fnid;
	TypeId						fn_tid;
	TypeId						obj_tid;
	std::vector<ConcreteGParam> gparams;
	std::vector<std::string>	params_name;
	std::vector<FnAddr>			fn_list; // 存放verify阶段确定的函数地址
	BuiltinFnCompileCallback	compile_cb;
	llvm::Function*				llvm_ir_fn;
};
// 运行时动态导入函数
struct DynamicFnInfo {
	TypeId					 fn_tid;
	int						 dynlib_instance_id;
	void*					 dynlib_fn;
	DynamicFnExecuteCallback execute_cb;
};

/*
 * 保存所有函数(包括方法)
 * TODO: 改成唯一fnid
 */
class FnTable {
public:
public:
	FnTable() {}

	Variable* CallFn(FnAddr addr, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args);

	TypeId		GetFnTypeId(FnAddr addr) const;
	TypeId		GetFnReturnTypeId(FnAddr addr) const;
	std::string GetFnId(FnAddr addr) const;

	FnAddr AddUserDefineFn(VerifyContext& ctx, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, AstNodeBlockStmt* body, std::string fnname);
	FnAddr AddBuiltinFn(VerifyContext& ctx, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, BuiltinFnCompileCallback compile_cb, std::string fnid);
	FnAddr AddDynamicFn(TypeId fn_tid, int dynlib_instance_id, void* dynlib_fn, DynamicFnExecuteCallback cb);

private:
	Variable* call_userDef_fn(UserDefFnInfo& info, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args);
	Variable* call_builtin_fn(BuiltinFnInfo& info, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args);
	Variable* call_dynamic_fn(DynamicFnInfo& info, ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args);

public:
	void Compile(CompileContext& cctx);

private:
	void compile_user_define_fn(CompileContext& cctx);
	void compile_builtin_fn(CompileContext& cctx);

private:
	std::vector<UserDefFnInfo> m_userdef_fn_table;
	std::vector<BuiltinFnInfo> m_builtin_fn_table;
	std::vector<DynamicFnInfo> m_dynamic_fn_table;
};
