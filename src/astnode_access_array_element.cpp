#include "astnode_access_array_element.h"
#include "astnode_constraint.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type_array.h"
#include "type_mgr.h"
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
	m_array_expr_is_tmp			= vr_obj.IsTmp();
	TypeInfo* ti				= g_typemgr.GetTypeInfo(obj_tid);

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
	var array = []int{1,2,3};
	var i=0;
	var e = array[i];

	var array = []int{ []int{1,2}, []int{3,4} };
	var i=0;
	var e = array[i];

	var array = []int{ []int{1,2}, []int{3,4} };
	var i=0;
	var j=1;
	var e = array[i][j];
 */
llvm::Value* AstNodeAccessArrayElement::Compile(CompileContext& cctx) {
	TypeInfo*	ti_array	  = g_typemgr.GetTypeInfo(m_array_expr->GetResultTypeId());
	llvm::Type* ir_type_array = ti_array->GetLLVMIRType(cctx);

	llvm::Type* ir_type_index = g_typemgr.GetTypeInfo(TYPE_ID_UINT64)->GetLLVMIRType(cctx);

	llvm::Value* array = m_array_expr->Compile(cctx);
	llvm::Value* index = m_index_expr->Compile(cctx);

	if (array->getType() == ir_type_array) {
		// array是一个保存数组数据的寄存器, 需要获取指针, 否则没法执行GEP
		llvm::Value* array_store_ptr = IRB.CreateAlloca(ir_type_array, nullptr, "store_array");
		IRB.CreateStore(array, array_store_ptr);
		array = array_store_ptr;
	} else if (array->getType() == ir_type_array->getPointerTo()) {
		// array是一个指向数组数据的指针
	} else {
		panicf("bug");
	}

	if (index->getType() == ir_type_index) {
		// index value就是u64值
	} else if (index->getType() == ir_type_index->getPointerTo()) {
		// index value是*u64, 需要load为u64
		index = IRB.CreateLoad(ir_type_index, index, "load_array_index");
	} else {
		panicf("bug");
	}

	return IRB.CreateGEP(nullptr, array, std::vector<llvm::Value*>{IRB.getInt64(0), index}, "array_indexed_element");
}
