#include "astnode_literal_str.h"
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

AstNodeLiteralStr::AstNodeLiteralStr(std::string value) {
	m_value = value;
}
VerifyContextResult AstNodeLiteralStr::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	m_compile_to_left_value = vparam.ExpectLeftValue();

	// 将字面值转换为特定类型
	// TODO 值域校验
	TypeId tid_Str	  = ctx.GetCurStack()->GetVariable("Str")->GetValueTid();
	TypeId target_tid = TYPE_ID_NONE;
	if (vparam.GetExpectResultTid() != TYPE_ID_INFER && vparam.GetExpectResultTid() != tid_Str) {
		panicf("type error");
	}
	// 找到构造函数Str(size u64, data *u8)
	TypeId				tid_ptr_u8 = g_typemgr.GetOrAddTypePointer(ctx, TYPE_ID_UINT8);
	std::vector<TypeId> constructor_args_tid{
		TYPE_ID_UINT64,
		tid_ptr_u8,
	};
	TypeInfo*				 ti_Str		  = g_typemgr.GetTypeInfo(tid_Str);
	std::vector<std::string> constructors = ti_Str->GetConstructor(ctx, "Str", constructor_args_tid);
	if (constructors.empty()) {
		panicf("no constructor found");
	} else if (constructors.size() == 1) {
		m_Str_constructor_fnid = constructors.at(0);
	} else {
		panicf("multiple constructor found");
	}

	m_result_typeid = tid_Str;
	return VerifyContextResult(m_result_typeid, nullptr);
}
Variable* AstNodeLiteralStr::Execute(ExecuteContext& ctx) {
	panicf("not imp");
}
AstNodeLiteralStr* AstNodeLiteralStr::DeepCloneT() {
	AstNodeLiteralStr* newone = new AstNodeLiteralStr(m_value);
	newone->Copy(*this);

	return newone;
}
CompileResult AstNodeLiteralStr::Compile(CompileContext& cctx) {
	llvm::Type*		element_type  = llvm::Type::getInt8Ty(IRC);
	llvm::Type*		constant_type = llvm::ArrayType::get(element_type, m_value.size());
	llvm::Constant* v_str		  = cctx.AddGlobalValue("str", constant_type, llvm::ConstantDataArray::getRaw(m_value, m_value.size(), element_type));
	llvm::Value*	v_ptr_u8	  = IRB.CreateBitCast(v_str, g_typemgr.GetTypeInfo(TYPE_ID_UINT8)->GetLLVMIRType(cctx)->getPointerTo());

	// 自动转换为Str类型
	TypeInfo*	 ti		 = g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Type*	 ir_type = ti->GetLLVMIRType(cctx);
	llvm::Value* inst	 = IRB.CreateAlloca(ir_type, nullptr, "Str");

	llvm::Function* Str_constructor = IRM.getFunction(m_Str_constructor_fnid);
	IRB.CreateCall(Str_constructor, std::vector<llvm::Value*>{
										inst,
										IRB.getInt64(m_value.size()),
										v_ptr_u8,
									});
	llvm::Value* result = inst;
	if (!m_compile_to_left_value) {
		result = IRB.CreateLoad(ir_type, inst); // 返回rvalue
	}
	return CompileResult().SetResult(result);
}
