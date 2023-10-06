#pragma once

#include "compile_context.h"
#include "define.h"
#include <llvm-12/llvm/IR/Function.h>

class AstNodeBlockStmt;
class AstNodeComplexFnDef;
class FnInfo;

typedef void (*BuiltinFnCompileCallback)(CompileContext& cctx, const FnInfo& fn_info);

class BuiltinFn {

public:
	static void register_builtin_fns(AstNodeBlockStmt& astnode_block_stmt);

	static void compile_nop(CompileContext& cctx, const FnInfo& fn_info);

	static void register_external_fn(CompileContext& cctx);

	// 基础类型的tostring方法的实现
	// fn T::tostring[](*T)Str#17  # T是integer/floating/bool
	static void compile_bfn_tostring(CompileContext& cctx, const FnInfo& fn_info);

private:
	// fn abort();
	static AstNodeComplexFnDef* register_bfn_abort();
	static void					compile_bfn_abort(CompileContext& cctx, const FnInfo& fn_info);

	// fn write(fd i32, data *u8, size u64) u64;
	static AstNodeComplexFnDef* register_bfn_write();
	static void					compile_bfn_write(CompileContext& cctx, const FnInfo& fn_info);
};
