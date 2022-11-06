#pragma once

#include "define.h"
#include "fntable.h"
#include "variable.h"
#include <vector>

class AstNode;
class VerifyContext;
class BuiltinFn;

// 指向内置函数的执行函数的指针
typedef Variable* (*BuiltinFnCallback)(BuiltinFn& builtin_fn, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);

// 指向内置函数的verify函数的指针
typedef void (*BuiltinFnVerify)(BuiltinFn& builtin_fn, VerifyContext& ctx);

/*
 * 内置函数
 */
struct BuiltinFn {
	BuiltinFn() {}
	BuiltinFn(TypeId obj_tid, BuiltinFnCallback callback, BuiltinFnVerify verify) {
		this->obj_tid  = obj_tid;
		this->callback = callback;
		this->verify   = verify;
	}
	BuiltinFn(TypeId obj_tid, BuiltinFnCallback callback, BuiltinFnVerify verify, int dynlib_instance_id, void* dynlib_fn) {
		this->obj_tid			 = obj_tid;
		this->callback			 = callback;
		this->verify			 = verify;
		this->dynlib_instance_id = dynlib_instance_id;
		this->dynlib_fn			 = dynlib_fn;
	}
	TypeId				 obj_tid;
	TypeId				 fn_tid;
	BuiltinFnCallback	 callback;
	BuiltinFnVerify		 verify;
	std::vector<FnAddr> fn_list; // 存放verify阶段确定的函数地址

	// 动态库加载得到的函数
	int	  dynlib_instance_id;
	void* dynlib_fn;

	void	  SetFnTid(TypeId fn_tid) { this->fn_tid = fn_tid; }
	void	  Verify(VerifyContext& ctx);
	Variable* Call(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);
	int		  GetDynLibInstanceId() const { return dynlib_instance_id; }
	void*	  GetDynLibFn() const { return dynlib_fn; }
};
