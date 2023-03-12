#pragma once
#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"

#include <llvm-12/llvm/IR/Function.h>
#include <string>
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

struct DynamicFnInfo;

// 指向内置函数的执行函数的指针
typedef Variable* (*DynamicFnExecuteCallback)(DynamicFnInfo& dynamic_fn, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);

// 用户定义函数
struct FnInfo {
	std::string					fnname;
	std::string					fnid;
	TypeId						fn_tid;
	TypeId						obj_tid;
	std::vector<ConcreteGParam> gparams;
	std::vector<std::string>	params_name;
	AstNodeBlockStmt*			body;
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

	TypeId GetFnTypeId(std::string fnid) const;
	TypeId GetFnReturnTypeId(std::string fnid) const;

	std::string AddUserDefineFn(VerifyContext& ctx, std::string fnname, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, AstNodeBlockStmt* body, bool is_nested);
	std::string AddBuiltinFn(VerifyContext& ctx, std::string fnname, TypeId fn_tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, BuiltinFnCompileCallback compile_cb);
	FnAddr		AddDynamicFn(TypeId fn_tid, int dynlib_instance_id, void* dynlib_fn, DynamicFnExecuteCallback cb);

public:
	void Compile(CompileContext& cctx);

private:
	void		compile_user_define_fn(CompileContext& cctx);
	void		compile_builtin_fn(CompileContext& cctx);
	std::string generate_fnid(std::string user_def_fnname, TypeId obj_tid, std::vector<ConcreteGParam> concrete_generic_params, TypeId fn_tid, bool is_nested);

private:
	std::vector<DynamicFnInfo> m_dynamic_fn_table;

	std::map<std::string, FnInfo> m_fn_table;

	std::map<std::string, int> m_nested_fnid_seed;
};
