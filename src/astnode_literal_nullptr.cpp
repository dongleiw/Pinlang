#include "astnode_literal_nullptr.h"
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

AstNodeLiteralNullPtr::AstNodeLiteralNullPtr() {
}
VerifyContextResult AstNodeLiteralNullPtr::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	m_compile_to_left_value = vparam.ExpectLeftValue();

	if (vparam.GetExpectReturnTid() == TYPE_ID_INFER) {
		panicf("type error");
	}
	TypeInfo* ti_expect = g_typemgr.GetTypeInfo(vparam.GetExpectResultTid());
	if (!ti_expect->IsPointer()) {
		panicf("can not cast `null` to type[%d:%s]", vparam.GetExpectResultTid(), ti_expect->GetName().c_str());
	}
	m_result_typeid = vparam.GetExpectResultTid();

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeLiteralNullPtr::Execute(ExecuteContext& ctx) {
	panicf("not imp");
}
AstNodeLiteralNullPtr* AstNodeLiteralNullPtr::DeepCloneT() {
	AstNodeLiteralNullPtr* newone = new AstNodeLiteralNullPtr();
	newone->Copy(*this);

	return newone;
}
CompileResult AstNodeLiteralNullPtr::Compile(CompileContext& cctx) {
	if (m_compile_to_left_value) {
		panicf("literal cannot be lvalue");
	}
	TypeInfo*	 ti		= g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Value* result = llvm::ConstantPointerNull::get((llvm::PointerType*)(ti->GetLLVMIRType(cctx)));
	return CompileResult().SetResult(result);
}
