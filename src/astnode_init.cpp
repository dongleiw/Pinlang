#include "astnode_init.h"
#include "astnode_type.h"
#include "compile_context.h"
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
#include <llvm-12/llvm/IR/Type.h>

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
CompileResult AstNodeInit::Compile(CompileContext& cctx) {
	// 获取类型
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_result_typeid);
	if (ti->IsArray()) {
		TypeInfoArray* ti_array		   = dynamic_cast<TypeInfoArray*>(ti);
		TypeInfo*	   ti_element	   = g_typemgr.GetTypeInfo(ti_array->GetElementType());
		llvm::Type*	   ir_type_array   = ti_array->GetLLVMIRType(cctx);
		llvm::Type*	   ir_type_element = ti_element->GetLLVMIRType(cctx);
		if (ti_array->IsStaticSize()) {
			// 静态大小数组是一个value type, 直接分配在stack上.
			llvm::Value* array = IRB.CreateAlloca(ti_array->GetLLVMIRType(cctx));
			for (size_t i = 0; i < m_elements.size(); i++) {
				assert(m_elements.at(i).attr_name.empty());
				CompileResult element_value = m_elements.at(i).attr_value->Compile(cctx);
				assert(element_value.GetResult()->getType() == ir_type_element);
				llvm::Value* pv_array_element = IRB.CreateConstGEP2_64(ir_type_array, array, 0, i); // 获取第`i`个数组元素的地址
				IRB.CreateStore(element_value.GetResult(), pv_array_element);						// 将初始化的值存储到数组元素位置
			}
			if (!m_compile_to_left_value) {
				array = IRB.CreateLoad(ir_type_array, array);
			}
			return CompileResult().SetResult(array);
		} else {
			// 数组大小是动态的. 是一个reference type, 需要在heap上分配内存. 还不支持
			panicf("not implemented yet");
		}
	} else if (ti->IsClass()) {
		llvm::Type*	 ir_type_class = ti->GetLLVMIRType(cctx);
		llvm::Value* class_inst	   = IRB.CreateAlloca(ir_type_class, nullptr, sprintf_to_stdstr("class_inst_%s", ti->GetName().c_str()));
		for (size_t i = 0; i < m_elements.size(); i++) {
			assert(!m_elements.at(i).attr_name.empty());
			CompileResult element_value = m_elements.at(i).attr_value->Compile(cctx);
			llvm::Value*  element_addr	= IRB.CreateConstGEP2_32(ir_type_class, class_inst, 0, (int)ti->GetFieldIndex(m_elements.at(i).attr_name));
			IRB.CreateStore(element_value.GetResult(), element_addr);
		}
		return CompileResult().SetResult(class_inst);
	} else {
		panicf("not implemented yet");
	}
}
