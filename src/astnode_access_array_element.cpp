#include "astnode_access_array_element.h"
#include "astnode_constraint.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type_array.h"
#include "type_mgr.h"
#include "variable.h"

VerifyContextResult AstNodeAccessArrayElement::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify access array element");

	if (vparam.ExpectLeftValue()) {
	}

	VerifyContextResult vr_obj	= m_array_expr->Verify(ctx, VerifyContextParam());
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
CompileResult AstNodeAccessArrayElement::Compile(VM& vm, FnInstructionMaker& maker) {
	TypeInfo* ti_array = g_typemgr.GetTypeInfo(m_array_tid);
	assert(ti_array->IsArray() && ti_array->IsValueType()); // 目前只支持值类型数组的下标访问. 引用类型的数组还不知道如何实现
	TypeInfo* ti_element = g_typemgr.GetTypeInfo(dynamic_cast<TypeInfoArray*>(ti_array)->GetElementType());

	TypeInfo* ti_u64 = g_typemgr.GetTypeInfo(TYPE_ID_UINT64);

	maker.AddComment(InstructionComment("access array element"));

	// 编译数组下标表达式
	CompileResult cr_idx = m_index_expr->Compile(vm, maker);
	assert(cr_idx.IsFnId() == false);

	RegisterId register_element_addr = vm.AllocGeneralRegister();
	// 下标的值 * 数组元素大小, 结果存储到register_element_addr中
	if (cr_idx.IsValue()) {
		maker.AddInstruction(new Instruction_mul_const<uint64_t, true, true>(maker, register_element_addr, cr_idx.GetRegisterId(), ti_element->GetMemSize()));
	} else {
		maker.AddInstruction(new Instruction_mul_const<uint64_t, true, false>(maker, register_element_addr, cr_idx.GetRegisterId(), ti_element->GetMemSize()));
	}
	vm.ReleaseGeneralRegister(cr_idx.GetRegisterId());
	if (!cr_idx.GetStackVarName().empty()) {
		maker.VarEnd(cr_idx.GetStackVarName());
	}

	// 编译数组表达式
	CompileResult cr_array = m_array_expr->Compile(vm, maker);
	assert(cr_array.IsFnId() == false && cr_array.IsValue() == false); // 数组表达式编译结果一定是一个存储内存地址的寄存器

	// 数组的内存地址 + 偏移地址, 结果存储到offset变量中. 这样就得到数组元素的内存地址了
	maker.AddInstruction(new Instruction_add<uint64_t, true, true, true>(maker, register_element_addr, cr_array.GetRegisterId(), register_element_addr));

	vm.ReleaseGeneralRegister(cr_array.GetRegisterId());
	if (!cr_array.GetStackVarName().empty()) {
		maker.VarEnd(cr_array.GetStackVarName());
	}

	return CompileResult(register_element_addr, false, "");
}
