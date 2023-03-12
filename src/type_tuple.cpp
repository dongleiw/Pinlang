#include "type_tuple.h"

#include <assert.h>
#include <map>
#include <utility>
#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_type.h"
#include "define.h"
#include "fntable.h"
#include "execute_context.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"

TypeInfoTuple::TypeInfoTuple(std::vector<TypeId> element_tids) {
	m_element_tids = element_tids;
	m_typegroup_id = TYPE_GROUP_ID_TUPLE;
	m_name		   = generate_name();

	std::vector<std::pair<std::string, TypeId>> fields;
	for (size_t i = 0; i < m_element_tids.size(); i++) {
		fields.push_back(std::pair(GetFieldName(i), m_element_tids.at(i)));
	}
	SetFields(fields);
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
std::string TypeInfoTuple::GetFieldName(int idx) {
	return std::string("f") + to_str(idx);
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
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("tostring", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		AddConstraint(constraint_tid, fns);
	}
	ctx.PopSTack();
}
