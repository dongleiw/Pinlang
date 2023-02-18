#include "astnode_init.h"
#include "astnode_type.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_array.h"
#include "type_class.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "verify_context.h"

VerifyContextResult AstNodeInit::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify init");

	// 获取类型
	TypeId tid = vparam.GetResultTid();
	if (m_type != nullptr) {
		tid = m_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
	}
	if (tid == TYPE_ID_INFER) {
		// 类型缺省, 只能依靠上下文推断
		panicf("bug");
	}
	TypeInfo* ti = g_typemgr.GetTypeInfo(tid);

	if (ti->IsArray()) {
		// 类型为数组, 检查所有元素类型是否正确
		TypeInfoArray* ti_array			 = dynamic_cast<TypeInfoArray*>(ti);
		TypeId		   array_element_tid = ti_array->GetElementType();
		for (auto& iter : m_elements) {
			if (!iter.attr_name.empty()) {
				panicf("attribute name is not allowed in array-init");
			}
			VerifyContextResult vresult = iter.attr_value->Verify(ctx, VerifyContextParam().SetResultTid(array_element_tid));
			if (vresult.GetResultTypeId() != array_element_tid) {
				panicf("element type[%d:%s] is wrong. should be type[%d:%s]", vresult.GetResultTypeId(), GET_TYPENAME_C(vresult.GetResultTypeId()),
					   array_element_tid, GET_TYPENAME_C(array_element_tid));
			}
		}
	} else if (ti->IsClass()) {
		// 类型为class. 检查所有属性类型是否正确
		TypeInfoClass* ti_class = dynamic_cast<TypeInfoClass*>(ti);
		for (auto& iter : m_elements) {
			if (iter.attr_name.empty()) {
				panicf("attribute name should be provided in class-init");
			}
			TypeId				attr_tid = ti_class->GetFieldType(iter.attr_name);
			VerifyContextResult vresult	 = iter.attr_value->Verify(ctx, VerifyContextParam().SetResultTid(attr_tid));
			if (vresult.GetResultTypeId() != attr_tid) {
				panicf("attr[%s] type[%d:%s] is wrong. should be type[%d:%s]", iter.attr_name.c_str(),
					   vresult.GetResultTypeId(), GET_TYPENAME_C(vresult.GetResultTypeId()),
					   attr_tid, GET_TYPENAME_C(attr_tid));
			}
		}
	} else {
		panicf("unexpected type[%d:%s]", tid, GET_TYPENAME_C(tid));
	}

	m_result_typeid = tid;
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeInit::Execute(ExecuteContext& ctx) {
	// 获取类型
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_result_typeid);

	if (ti->IsArray()) {
		std::vector<Variable*> array_elements;
		for (auto& iter : m_elements) {
			Variable* v = iter.attr_value->Execute(ctx);
			array_elements.push_back(v);
		}
		return new Variable(m_result_typeid, array_elements);
	} else if (ti->IsClass()) {
		Variable* class_obj = new Variable(m_result_typeid);

		// 注意: 由于允许缺省, 可能存在一些字段没有显式提供初始值
		std::map<std::string, Variable*> fields;

		// 处理显式指定的字段
		for (auto& iter : m_elements) {
			Variable* v			   = iter.attr_value->Execute(ctx);
			fields[iter.attr_name] = v;
		}

		// 处理缺省的字段
		for (auto& iter : ti->GetField()) {
			if (fields.find(iter.name) == fields.end()) {
				fields[iter.name] = new Variable(iter.tid);
			}
		}

		class_obj->InitField(fields);
		return class_obj;
	} else {
		panicf("bug");
	}
}
AstNodeInit* AstNodeInit::DeepCloneT() {
	AstNodeInit* newone = new AstNodeInit();
	newone->m_type		= m_type == nullptr ? nullptr : m_type->DeepCloneT();
	for (auto iter : m_elements) {
		newone->m_elements.push_back(iter.DeepClone());
	}
	return newone;
}
CompileResult AstNodeInit::Compile(VM& vm, FnInstructionMaker& maker) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_result_typeid);

	Var var_result = maker.TmpVarBegin("init", ti->GetMemSize());

	// 一个保存当前目标内存地址的寄存器
	RegisterId register_addr = vm.AllocGeneralRegister();
	maker.AddInstruction(new Instruction_add_const<uint64_t, true, true>(maker, register_addr, REGISTER_ID_STACK_FRAME, var_result.mem_addr.relative_addr,
																		 sprintf_to_stdstr("get addr of init-expr-result")));

	if (ti->IsArray()) {
		// 是数组初始化表达式. 将每个元素都编译到目标内存地址的相应偏移位置
		TypeInfoArray* ti_array	  = dynamic_cast<TypeInfoArray*>(ti);
		TypeInfo*	   ti_element = g_typemgr.GetTypeInfo(ti_array->GetElementType());
		if (ti->IsValueType()) {
			for (size_t i = 0; i < m_elements.size(); i++) {
				CompileResult cr_element = m_elements.at(i).attr_value->Compile(vm, maker);
				if (cr_element.IsFnId()) {
					panicf("not implemented");
				} else {
					if (cr_element.IsValue()) {
						maker.AddInstruction(new Instruction_store_register(maker, register_addr, cr_element.GetRegisterId(), ti_element->GetMemSize()));
					} else {
						maker.AddInstruction(new Instruction_memcpy(maker, register_addr, cr_element.GetRegisterId(), ti_element->GetMemSize()));
					}
					vm.ReleaseGeneralRegister(cr_element.GetRegisterId());
					if (!cr_element.GetStackVarName().empty()) {
						maker.VarEnd(cr_element.GetStackVarName());
					}
				}

				if (i + 1 == m_elements.size()) {
					break;
				}
				// 寄存器中保存的内存地址指向下一个数组元素
				maker.AddInstruction(new Instruction_add_const<uint64_t, true, true>(maker, register_addr, register_addr, ti_element->GetMemSize(),
																					 sprintf_to_stdstr("get addr of init-element[%d]", i+1)));
			}
		} else {
			panicf("not implemented");
		}
	} else if (ti->IsClass()) {
		panicf("not implemented");
	} else {
		panicf("bug");
	}

	maker.AddInstruction(new Instruction_add_const<uint64_t, true, true>(maker, register_addr, REGISTER_ID_STACK_FRAME, var_result.mem_addr.relative_addr,
																		 sprintf_to_stdstr("get addr of init-expr-result")));
	return CompileResult(register_addr, false, var_result.var_name);
}
