#pragma once

#include "compile_context.h"
#include "define.h"
#include <llvm-12/llvm/IR/Function.h>

class AstNodeBlockStmt;
class AstNodeComplexFnDef;

typedef void (*BuiltinFnCompileCallback)(CompileContext& cctx, std::string fnid);

class BuiltinFn {

public:
	static void register_builtin_fns(AstNodeBlockStmt& astnode_block_stmt);

	static void compile_nop(CompileContext& cctx, std::string fnid);

private:
	static AstNodeComplexFnDef* register_bfn_abort();
	static void					compile_bfn_abort(CompileContext& cctx, std::string fnid);
};
