#include "type_tuple.h"

#include <assert.h>
#include <map>
#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_type.h"
#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"

static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(args.size() == 0);
	TypeInfoTuple*		ti_tuple	 = dynamic_cast<TypeInfoTuple*>(g_typemgr.GetTypeInfo(thisobj->GetTypeId()));
	std::vector<TypeId> element_tids = ti_tuple->GetElementTids();

	std::string s = "(";
	for (size_t i = 0; i < element_tids.size(); i++) {
		std::string field_name = TypeInfoTuple::GetFieldName(i);
		Variable* element = thisobj->GetFieldValue(field_name);
		// 调用tostring方法来转换为str
		// TODO 目前得先获取MethodIndex然后调用
		TypeInfo*	ti_element	 = g_typemgr.GetTypeInfo(element_tids.at(i));
		MethodIndex method_index = ti_element->GetMethodIdx("tostring[]()str");
		Variable*	str_e		 = element->CallMethod(ctx, method_index, std::vector<Variable*>());
		s += str_e->GetValueStr();
		if (i + 1 < element_tids.size()) {
			s += ",";
		}
	}
	s += ")";
	return new Variable(s);
}

TypeInfoTuple::TypeInfoTuple(std::vector<TypeId> element_tids) {
	m_element_tids = element_tids;
	m_typegroup_id = TYPE_GROUP_ID_TUPLE;
	m_name		   = generate_name();

	set_fields();
}
std::string TypeInfoTuple::generate_name() {
	std::string s = "tuple(";
	char		buf[8];
	for (size_t i = 0; i < m_element_tids.size(); i++) {
		snprintf(buf, sizeof(buf), "%d", m_element_tids.at(i));
		s = s + buf + ":" + GET_TYPENAME(m_element_tids.at(i));
		if (i + 1 != m_element_tids.size()) {
			s += ",";
		}
	}
	s += ")";

	return s;
}
void TypeInfoTuple::set_fields() {
	for (size_t i = 0; i < m_element_tids.size(); i++) {
		AddField(GetFieldName(i), m_element_tids.at(i));
	}
}
std::string TypeInfoTuple::GetFieldName(int idx) {
	return std::string("f") + int_to_str(idx);
}
void TypeInfoTuple::InitBuiltinMethods(VerifyContext& ctx) {
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
	ctx.PopSTack();
}
