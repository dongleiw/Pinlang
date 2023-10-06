#include "astnode_access_array_element.h"
#include "astnode_constraint.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type_array.h"
#include "type_mgr.h"
#include "type_pointer.h"
#include "variable.h"
#include <llvm-12/llvm/IR/DerivedTypes.h>
#include <llvm-12/llvm/IR/Type.h>
#include <llvm-12/llvm/Support/Casting.h>
#include <llvm-12/llvm/Support/raw_ostream.h>

VerifyContextResult AstNodeAccessArrayElement::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	log_debug("verify access array element");

	VerifyContextResult vr_obj = m_array_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(true));
	m_array_tid				   = vr_obj.GetResultTypeId();
	TypeInfo* ti			   = g_typemgr.GetTypeInfo(m_array_tid);

	TypeId index_tid = m_index_expr->Verify(ctx, VerifyContextParam().SetExpectResultTid(TYPE_ID_UINT64)).GetResultTypeId();
	if (index_tid != TYPE_ID_UINT64) {
		panicf("index type[%d:%s] is not u64", index_tid, GET_TYPENAME_C(index_tid));
	}

	// 下标访问同时支持两种类型
	//		数组下标访问
	//			var array = [2]i32{1,2}; // array是一个数组
	//			var e = array[0]; // array是数组, 需要获取这个数组的内存地址, 才能计算偏移量得到元素的内存地址
	//		指向元素的指针的下标访问
	//			var a = [2]i32{1,2};
	//			var array = &(a[0]); // array.type = *i32
	//			var e = array[0]; // array是指向数组元素的指针, 需要得到这个指针的值(也就是数组元素的内存地址)
	//
	//		那么: (&([2]i32{1,2}[0]))[0] ??
	if (ti->IsArray()) {
		// 数组类型需要编译为lvalue, 也就是数组的地址
		m_array_expr->SetCompileToLeftValue();
	} else if (ti->IsPointer()) {
		// 指针类型需要编译为rvalue, 也就是指针的值(内存地址)
		m_array_expr->SetCompileToRightValue();
	} else {
		panicf("bug");
	}

	std::vector<std::string> method_indexs = ti->GetConstraintMethod(ctx, "Index", "index", std::vector<TypeId>{index_tid}, TYPE_ID_INFER);
	if (method_indexs.empty()) {
		panicf("type[%d:%s] not implement constraint Index", m_array_tid, GET_TYPENAME_C(m_array_tid));
	} else if (method_indexs.size() > 1) {
		panicf("type[%d:%s] has multiple method[index] of constraint[Index]", m_array_tid, GET_TYPENAME_C(m_array_tid));
	} else {
	}

	std::string fnid		= method_indexs.at(0);
	m_result_typeid			= ctx.GetFnTable().GetFnReturnTypeId(fnid);
	m_compile_to_left_value = vparam.ExpectLeftValue();

	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
AstNodeAccessArrayElement* AstNodeAccessArrayElement::DeepCloneT() {
	AstNodeAccessArrayElement* newone = new AstNodeAccessArrayElement();
	newone->Copy(*this);

	newone->m_array_expr = m_array_expr->DeepClone();
	newone->m_index_expr = m_index_expr->DeepClone();

	return newone;
}
/*
 */
CompileResult AstNodeAccessArrayElement::Compile(CompileContext& cctx) {
	CompileResult cr_array = m_array_expr->Compile(cctx);
	CompileResult cr_index = m_index_expr->Compile(cctx);

	TypeInfo*	 ti_array			   = g_typemgr.GetTypeInfo(m_array_expr->GetResultTypeId());
	TypeInfo*	 ti_array_element	   = g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Type*	 ir_type_array_element = ti_array_element->GetLLVMIRType(cctx);
	llvm::Value* result;
	if (ti_array->IsArray()) {
		// array是数组
		assert(cr_array.GetResult()->getType() == ti_array->GetLLVMIRType(cctx)->getPointerTo());
		llvm::Value* ptr_to_array_element = IRB.CreateGEP(nullptr, cr_array.GetResult(), std::vector<llvm::Value*>{IRB.getInt64(0), cr_index.GetResult()}, "array_indexed_element");
		if (m_compile_to_left_value) {
			result = ptr_to_array_element;
		} else {
			result = IRB.CreateLoad(ir_type_array_element, ptr_to_array_element);
		}
	} else if (ti_array->IsPointer()) {
		// array是指向数组元素的指针
		TypeInfoPointer* ti_pointer	   = dynamic_cast<TypeInfoPointer*>(ti_array);
		TypeInfo*		 ti_pointee	   = g_typemgr.GetTypeInfo(ti_pointer->GetPointeeTid());
		llvm::Type*		 ir_type_array = llvm::ArrayType::get(ti_pointee->GetLLVMIRType(cctx), 0);
		llvm::Value*	 cast_array	   = IRB.CreateBitCast(cr_array.GetResult(), ir_type_array->getPointerTo());

		llvm::Value* ptr_to_array_element = IRB.CreateGEP(nullptr, cast_array, std::vector<llvm::Value*>{IRB.getInt64(0), cr_index.GetResult()}, "array_indexed_element");
		if (m_compile_to_left_value) {
			result = ptr_to_array_element;
		} else {
			result = IRB.CreateLoad(ir_type_array_element, ptr_to_array_element);
		}
	} else {
		panicf("bug");
	}
	return CompileResult().SetResult(result);
}
