#include "astnode_literal_bool.h"
#include "astnode.h"
#include "astnode_type.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "verify_context.h"
#include <cstdint>
#include <llvm-12/llvm/ADT/APFloat.h>
#include <llvm-12/llvm/ADT/APInt.h>
#include <llvm-12/llvm/IR/Constant.h>
#include <llvm-12/llvm/IR/Constants.h>
#include <llvm-12/llvm/IR/DerivedTypes.h>
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/Support/Casting.h>
#include <vector>

AstNodeLiteralBool::AstNodeLiteralBool(bool value) {
	m_value = value;
}
VerifyContextResult AstNodeLiteralBool::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	m_compile_to_left_value = vparam.ExpectLeftValue();

	// 将字面值转换为特定类型
	// TODO 值域校验
	TypeId target_tid = TYPE_ID_NONE;
	if (vparam.GetExpectResultTid() != TYPE_ID_INFER && vparam.GetExpectResultTid() != TYPE_ID_BOOL) {
		panicf("type error");
	}

	m_result_typeid = TYPE_ID_BOOL;
	return VerifyContextResult(m_result_typeid, new Variable(bool(m_value)));
}
Variable* AstNodeLiteralBool::Execute(ExecuteContext& ctx) {
	panicf("not imp");
}
AstNodeLiteralBool* AstNodeLiteralBool::DeepCloneT() {
	AstNodeLiteralBool* newone = new AstNodeLiteralBool(m_value);
	newone->Copy(*this);

	return newone;
}
CompileResult AstNodeLiteralBool::Compile(CompileContext& cctx) {
	TypeInfo*	ti		= g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Type* ir_type = ti->GetLLVMIRType(cctx);

	llvm::Value* result = nullptr;
	result				= llvm::ConstantInt::get(IRC, llvm::APInt(1, m_value ? 1 : 0, false));
	if (m_compile_to_left_value) {
		llvm::Value* allocated_inst = IRB.CreateAlloca(ir_type);
		result						= IRB.CreateStore(result, allocated_inst);
	}

	return CompileResult().SetResult(result);
}
