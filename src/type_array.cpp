#include "type_array.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_fncall.h"
#include "builtin_fn.h"
#include "define.h"
#include "fntable.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "variable_table.h"

#include <assert.h>
#include <bits/stdint-uintn.h>
#include <llvm-12/llvm/IR/DerivedTypes.h>
#include <vector>

TypeInfoArray::TypeInfoArray(TypeId element_tid, uint64_t size) {
	m_element_tid  = element_tid;
	m_typegroup_id = TYPE_GROUP_ID_ARRAY;
	m_static_size  = size;

	TypeInfo* element_ti = g_typemgr.GetTypeInfo(element_tid);

	if (m_static_size > 0) {
		m_original_name = "[" + to_str(size) + "]";
		m_is_value_type = true;
	} else {
		// 数组大小不是编译期确定. 变量的值是一个指向数组实际数据的指针
		m_original_name = "[]";
	}
	m_original_name += element_ti->GetName();
}
void TypeInfoArray::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	// 手动实现ToString约束
	//{
	//	std::vector<AstNodeComplexFnDef*> fns;
	//	{
	//		std::vector<AstNodeComplexFnDef::Implement> implements;
	//		{
	//			std::vector<ParserGenericParam> gparams;
	//			std::vector<ParserParameter>	params;
	//			AstNodeType*					return_type = new AstNodeType();
	//			return_type->InitWithIdentifier("str");
	//			implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_tostring_verify, builtin_fn_callback_tostring));
	//		}
	//		AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("tostring", implements);
	//		astnode_complex_fndef->Verify(ctx, VerifyContextParam());
	//		fns.push_back(astnode_complex_fndef);
	//	}

	//	AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
	//	TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
	//	AddConstraint(constraint_tid, fns);

	//	GetConstraintMethod(ctx, "ToString", "tostring", std::vector<TypeId>()); // 触发tostring函数的实例化
	//}
	// 手动实现Index约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* index_type = new AstNodeType();
					index_type->InitWithIdentifier("u64");
					params.push_back({ParserParameter{
						.name = "a",
						.type = index_type,
					}});
				}

				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithTargetTypeId(m_element_tid);

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}

			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("index", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());

			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Index")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_element_tid}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	ctx.PopStack();
}
llvm::Type* TypeInfoArray::GetLLVMIRType(CompileContext& cctx) {
	if (IsStaticSize()) {
		// 静态大小的数组
		TypeInfo* ti_element = g_typemgr.GetTypeInfo(m_element_tid);
		return llvm::ArrayType::get(ti_element->GetLLVMIRType(cctx), m_static_size);
	} else {
		// 动态大小的数组, 其实是指向元素的指针
		TypeInfo* ti_element = g_typemgr.GetTypeInfo(m_element_tid);
		return ti_element->GetLLVMIRType(cctx)->getPointerTo();
	}
}
void TypeInfoArray::ConstructDefault(CompileContext& cctx, llvm::Value* obj) {
}
