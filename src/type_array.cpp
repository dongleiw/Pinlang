#include "type_array.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "type_mgr.h"

#include <assert.h>
#include <vector>

static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj != nullptr && args.size() == 0);
	std::string	   s  = "{,";
	TypeInfoArray* ti = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(thisobj->GetTypeId()));
	for (auto e : thisobj->GetValueArray()) {
		// 调用tostring方法来转换为str
		// TODO 目前得先获取MethodIndex然后调用
		MethodIndex method_index = ti->GetMethodIdx("tostring");
		Variable*	str_e		 = thisobj->CallMethod(ctx, method_index, std::vector<Variable*>());
		s += str_e->GetValueStr() + ",";
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
		AstNodeConstraint*				 constraint		= ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId							 constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		std::map<std::string, Function*> methods;

		TypeId	  tid		= g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{}, TYPE_ID_STR);
		Function* f			= new Function(tid, builtin_fn_tostring);
		methods["tostring"] = f;

		AddConstraint(constraint_tid, methods);
	}
}
