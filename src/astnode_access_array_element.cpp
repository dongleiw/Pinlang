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
	log_debug("verify access array element");

	if (vparam.ExpectLeftValue()) {
	}

	VerifyContextResult vr_obj	= m_array_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(true));
	TypeId				obj_tid = vr_obj.GetResultTypeId();
	TypeInfo*			ti		= g_typemgr.GetTypeInfo(obj_tid);

	TypeId index_tid = m_index_expr->Verify(ctx, VerifyContextParam().SetResultTid(TYPE_ID_UINT64)).GetResultTypeId();
	if (index_tid != TYPE_ID_UINT64) {
		panicf("index type is not int");
	}

	std::vector<MethodIndex> method_indexs = ti->GetConstraintMethod(ctx, "Index", "index", std::vector<TypeId>{index_tid});
	if (method_indexs.empty()) {
		panicf("type[%d:%s] not implement constraint Index", obj_tid, GET_TYPENAME_C(obj_tid));
	} else if (method_indexs.size() > 1) {
		panicf("type[%d:%s] has multiple method[index] of constraint[Index]", obj_tid, GET_TYPENAME_C(obj_tid));
	} else {
	}

	MethodIndex method_index = method_indexs.at(0);
	m_fn_addr				 = ti->GetMethodByIdx(method_index);
	m_array_tid				 = obj_tid;
	m_result_typeid			 = ctx.GetFnTable().GetFnReturnTypeId(m_fn_addr);
	m_compile_to_left_value	 = vparam.ExpectLeftValue();

	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeAccessArrayElement::Execute(ExecuteContext& ctx) {
	Variable* assign_value = ctx.GetAssignValue();
	ctx.SetAssignValue(nullptr);

	Variable* v_array = m_array_expr->Execute(ctx);
	Variable* v_index = m_index_expr->Execute(ctx);

	if (assign_value != nullptr) {
		v_array->SetValueArrayElement(v_index->GetValueUInt64(), assign_value);
		return nullptr;
	} else {
		std::vector<Variable*> args{v_index};
		return ctx.GetFnTable().CallFn(m_fn_addr, ctx, v_array, args);
	}
}
AstNodeAccessArrayElement* AstNodeAccessArrayElement::DeepCloneT() {
	AstNodeAccessArrayElement* newone = new AstNodeAccessArrayElement();

	newone->m_array_expr = m_array_expr->DeepClone();
	newone->m_index_expr = m_index_expr->DeepClone();

	return newone;
}
/*
 */
llvm::Value* AstNodeAccessArrayElement::Compile(CompileContext& cctx) {
	llvm::Value* array = m_array_expr->Compile(cctx);
	llvm::Value* index = m_index_expr->Compile(cctx);

	TypeInfo*	ti_array			  = g_typemgr.GetTypeInfo(m_array_expr->GetResultTypeId());
	TypeInfo*	ti_array_element	  = g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Type* ir_type_array_element = ti_array_element->GetLLVMIRType(cctx);
	if (ti_array->IsArray()) {
		/*
		 * `array`是一个指向数组的指针
		 * var a = [2]i32{1,2};
		 * var array = &a; // *[2]i32
		 */
		assert(array->getType() == ti_array->GetLLVMIRType(cctx)->getPointerTo());
		if (m_compile_to_left_value) {
			return IRB.CreateGEP(nullptr, array, std::vector<llvm::Value*>{IRB.getInt64(0), index}, "array_indexed_element");
		} else {
			llvm::Value* element_ptr = IRB.CreateGEP(nullptr, array, std::vector<llvm::Value*>{IRB.getInt64(0), index}, "array_indexed_element");
			return IRB.CreateLoad(ir_type_array_element, element_ptr);
		}
	} else if (ti_array->IsPointer()) {
		/*
		 * `array`是一个指向数组元素的指针
		 * var a = [2]i32{1,2};
		 * var array = &(array[0]); // *i32
		 *
		 * 将类型为*i32的指针转换为*[0]i32
		 */
		TypeInfoPointer* ti_pointer	   = dynamic_cast<TypeInfoPointer*>(ti_array);
		TypeInfo*		 ti_pointee	   = g_typemgr.GetTypeInfo(ti_pointer->GetPointeeTid());
		llvm::Type*		 ir_type_array = llvm::ArrayType::get(ti_pointee->GetLLVMIRType(cctx), 0);
		llvm::Value*	 cast_array	   = IRB.CreateBitCast(array, ir_type_array->getPointerTo());
		if (m_compile_to_left_value) {
			return IRB.CreateGEP(nullptr, cast_array, std::vector<llvm::Value*>{IRB.getInt64(0), index}, "array_indexed_element");
		} else {
			llvm::Value* element_ptr = IRB.CreateGEP(nullptr, cast_array, std::vector<llvm::Value*>{IRB.getInt64(0), index}, "array_indexed_element");
			return IRB.CreateLoad(ir_type_array_element, element_ptr);
		}
	} else {
		panicf("bug");
	}
}
