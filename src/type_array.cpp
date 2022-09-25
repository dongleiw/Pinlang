#include "type_array.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "type_mgr.h"

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
		MethodIndex method_index = ti_element->GetMethodIdx("tostring");
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
	// 实现constraint ToString
	{
		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});

		TypeInfo* array_ti = g_typemgr.GetTypeInfo(GetTypeId());
		if (!array_ti->MatchConstraint(constraint_tid)) {
			// 已经实现过了. TODO 不应该在这里过滤
			TypeInfo* element_ti = g_typemgr.GetTypeInfo(m_element_tid);
			if (element_ti->MatchConstraint(constraint_tid)) {
				// 只有在数组元素类型实现了ToString约束时, 才自动给数组实现ToString约束
				std::map<std::string, Function*> methods;

				TypeId	  tid		= g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{}, TYPE_ID_STR);
				Function* f			= new Function(tid, std::vector<ConcreteGParam>(), builtin_fn_tostring);
				methods["tostring"] = f;

				AddConstraint(constraint_tid, methods);
			}
		}
	}
}
