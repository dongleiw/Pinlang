#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include <llvm-12/llvm/IR/Function.h>
#include <string>

#include "astnode_complex_fndef.h"

void BuiltinFn::register_builtin_fns(AstNodeBlockStmt& astnode_block_stmt) {
	astnode_block_stmt.AddChildStmt(register_bfn_abort());
}
AstNodeComplexFnDef* BuiltinFn::register_bfn_abort() {
	const std::string				fnid = "abort";
	std::vector<ParserGenericParam> gparams;
	std::vector<ParserParameter>	params{};

	std::vector<AstNodeComplexFnDef::Implement> implements;
	implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, nullptr, BuiltinFn::compile_bfn_abort));
	return new AstNodeComplexFnDef("abort", implements);
}
void BuiltinFn::compile_bfn_abort(CompileContext& cctx, std::string fnid) {
	llvm::FunctionType* libc_abort_fn_type = llvm::FunctionType::get(llvm::Type::getVoidTy(IRC), std::vector<llvm::Type*>{}, false);
	llvm::Function*		libc_abort_fn	   = llvm::Function::Create(libc_abort_fn_type, llvm::Function::ExternalLinkage, "abort", IRM);

	cctx.AddNamedValue(fnid, libc_abort_fn);
}
void BuiltinFn::compile_nop(CompileContext& cctx, std::string fnid) {
}
