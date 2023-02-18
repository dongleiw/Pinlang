#include "type_array.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_fncall.h"
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

//static Variable* builtin_fn_callback_tostring(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
//	assert(thisobj != nullptr && args.size() == 0 && builtin_fn_info.fn_list.size() == 1);
//	std::string s = "{";
//
//	FnAddr element_tostring_method_addr = builtin_fn_info.fn_list.at(0);
//
//	int array_size = thisobj->GetValueArraySize();
//	for (int i = 0; i < array_size; i++) {
//		Variable* element = thisobj->GetValueArrayElement(i);
//		// 调用tostring方法来转换为str
//		Variable* str_e = ctx.GetFnTable().CallFn(element_tostring_method_addr, ctx, element, std::vector<Variable*>());
//		s += str_e->GetValueStr();
//		if (i + 1 < array_size) {
//			s += ",";
//		}
//	}
//	s += "}";
//	return new Variable(s);
//}
//static void builtin_fn_tostring_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
//	TypeInfoArray* ti_array		= dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(builtin_fn_info.obj_tid));
//	TypeInfo*	   ti_element	= g_typemgr.GetTypeInfo(ti_array->GetElementType());
//	MethodIndex	   method_index = ti_element->GetMethodIdx("tostring[]()str");
//	FnAddr		   fn_addr		= ti_element->GetMethodByIdx(method_index);
//	builtin_fn_info.fn_list.push_back(fn_addr);
//}

//static Variable* builtin_fn_size_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
//	assert(thisobj != nullptr && args.size() == 0);
//	return new Variable(thisobj->GetValueArraySize());
//}
static void builtin_fn_size_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
}

static Variable* builtin_fn_index_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj != nullptr && g_typemgr.GetTypeInfo(thisobj->GetTypeId())->IsArray() && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_UINT64);

	uint64_t index = args.at(0)->GetValueUInt64();
	return thisobj->GetValueArrayElement(index);
}
static void builtin_fn_index_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
}

TypeInfoArray::TypeInfoArray(TypeId element_tid, uint64_t size) {
	m_element_tid  = element_tid;
	m_typegroup_id = TYPE_GROUP_ID_ARRAY;
	m_static_size  = size;

	TypeInfo* element_ti = g_typemgr.GetTypeInfo(element_tid);

	if (m_static_size > 0) {
		// 数组大小编译期确定. 变量的值就是数组元素
		m_mem_size		 = size * element_ti->GetMemSize();
		m_mem_align_size = element_ti->GetMemAlignSize();
		m_name			 = "[" + to_str(size) + "]";
		m_is_value_type	 = true;
	} else {
		// 数组大小不是编译期确定. 变量的值是一个指向数组实际数据的指针
		assert(sizeof(void*) == 8);
		m_mem_size		 = 8;
		m_mem_align_size = 8;
		m_name			 = "[]";
	}
	m_name += element_ti->GetName();
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

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_index_verify));
			}

			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("index", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());

			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Index")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_element_tid});
		AddConstraint(constraint_tid, fns);
	}
	// 增加内置方法
	{
		std::vector<AstNodeComplexFnDef*> fns;
		// 增加Size()int
		//{
		//	std::vector<AstNodeComplexFnDef::Implement> implements;
		//	{
		//		std::vector<ParserGenericParam> gparams;
		//		std::vector<ParserParameter>	params;
		//		AstNodeType*					return_type = new AstNodeType();
		//		return_type->InitWithIdentifier("int");
		//		implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_size_verify, builtin_fn_size_execute));
		//	}
		//	AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("Size", implements);
		//	astnode_complex_fndef->Verify(ctx, VerifyContextParam());
		//	fns.push_back(astnode_complex_fndef);
		//}
		AddConstraint(CONSTRAINT_ID_NONE, fns);
	}
	ctx.PopSTack();
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
