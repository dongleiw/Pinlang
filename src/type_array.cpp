#include "type_array.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "type_mgr.h"
#include "variable.h"
#include "variable_table.h"

#include <assert.h>
#include <vector>

static Variable* builtin_fn_tostring(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj != nullptr && args.size() == 0);
	std::string	   s		  = "{";
	TypeInfoArray* ti_array	  = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(thisobj->GetTypeId()));
	TypeInfo*	   ti_element = g_typemgr.GetTypeInfo(ti_array->GetElementType());

	int array_size = thisobj->GetValueArraySize();
	for (int i = 0; i < array_size; i++) {
		Variable* element = thisobj->GetValueArrayElement(i);
		// 调用tostring方法来转换为str
		// TODO 目前得先获取MethodIndex然后调用
		MethodIndex method_index = ti_element->GetMethodIdx("tostring[]()str");
		Variable*	str_e		 = element->CallMethod(ctx, method_index, std::vector<Variable*>());
		s += str_e->GetValueStr();
		if (i + 1 < array_size) {
			s += ",";
		}
	}
	s += "}";
	return new Variable(s);
}
static Variable* builtin_fn_size(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj != nullptr && args.size() == 0);
	return new Variable(thisobj->GetValueArraySize());
}

static Variable* builtin_fn_index(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj != nullptr && g_typemgr.GetTypeInfo(thisobj->GetTypeId())->IsArray() && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_INT32);

	int32_t index = args.at(0)->GetValueInt32();
	return thisobj->GetValueArrayElement(index);
}

TypeInfoArray::TypeInfoArray(TypeId element_tid) {
	m_element_tid	 = element_tid;
	m_name			 = "[]" + GET_TYPENAME(element_tid);
	m_typegroup_id	 = TYPE_GROUP_ID_ARRAY;
	m_mem_size		 = 8;
	m_mem_align_size = 8;
}
void TypeInfoArray::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	// 手动实现ToString约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				AstNodeType*					return_type = new AstNodeType();
				return_type->InitWithIdentifier("str");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_tostring));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("tostring", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		AddConstraint(constraint_tid, fns);

		GetConstraintMethod(ctx, "ToString", "tostring", std::vector<TypeId>()); // 触发tostring函数的实例化
	}
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
					index_type->InitWithIdentifier("i32");
					params.push_back({ParserParameter{
						.name = "a",
						.type = index_type,
					}});
				}

				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithTargetTypeId(m_element_tid);

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_index));
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
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				AstNodeType*					return_type = new AstNodeType();
				return_type->InitWithIdentifier("int");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, nullptr, builtin_fn_size));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("Size", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}
		AddConstraint(CONSTRAINT_ID_NONE, fns);
	}
	ctx.PopSTack();
}
