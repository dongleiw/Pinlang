#include "type_array.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "type_mgr.h"
#include "variable_table.h"

#include <assert.h>
#include <vector>

static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj != nullptr && args.size() == 0);
	std::string			   s		  = "{";
	TypeInfoArray*		   ti_array	  = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(thisobj->GetTypeId()));
	TypeInfo*			   ti_element = g_typemgr.GetTypeInfo(ti_array->GetElementType());
	std::vector<Variable*> elements	  = thisobj->GetValueArray();
	for (size_t i = 0; i < elements.size(); i++) {
		Variable* element = elements.at(i);
		// 调用tostring方法来转换为str
		// TODO 目前得先获取MethodIndex然后调用
		MethodIndex method_index = ti_element->GetMethodIdx("tostring[]()str");
		Variable*	str_e		 = element->CallMethod(ctx, method_index, std::vector<Variable*>());
		s += str_e->GetValueStr();
		if (i + 1 < elements.size()) {
			s += ",";
		}
	}
	s += "}";
	return new Variable(s);
}

TypeInfoArray::TypeInfoArray(TypeId element_tid) {
	m_element_tid  = element_tid;
	m_name		   = "[]" + GET_TYPENAME(element_tid);
	m_typegroup_id = TYPE_GROUP_ID_ARRAY;
}
void TypeInfoArray::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	{
		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		TypeInfo*		   array_ti		  = g_typemgr.GetTypeInfo(GetTypeId());
		if (!array_ti->MatchConstraint(constraint_tid)) {
			TypeInfo* element_ti = g_typemgr.GetTypeInfo(m_element_tid);
			if (element_ti->MatchConstraint(constraint_tid)) {
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

				AddConstraint(constraint_tid, fns);

				GetConcreteMethod(ctx, "tostring", std::vector<TypeId>(), TYPE_ID_STR);
			}
		}
	}
	ctx.PopSTack();
}
