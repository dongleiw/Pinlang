#include "astnode_restriction.h"
#include "astnode_type.h"
#include "define.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_restriction.h"
#include "variable.h"

#include "log.h"
#include "variable_table.h"
#include "verify_context.h"
#include <cassert>

VerifyContextResult AstNodeRestriction::Verify(VerifyContext& ctx) {
	log_debug("verify restriction");

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_name, new Variable(this));
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeRestriction::Execute(ExecuteContext& ctx) {
	return nullptr;
}
TypeId AstNodeRestriction::Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params) const {
	assert(HasGenericParam());
	assert(concrete_params.size() == m_generic_params.size());

	// 泛型名映射到实际类型id
	VariableTable* vt = new VariableTable();
	for (size_t i = 0; i < m_generic_params.size(); i++) {
		vt->AddVariable(m_generic_params.at(i), new Variable(concrete_params.at(i)));
	}

	// 添加一个临时vt, 避免污染现有vt
	ctx.GetCurStack()->EnterBlock(vt);

	// 遍历所有rule模板, 生成实际类型
	std::vector<TypeInfoRestriction::Rule> concrete_rules;
	for (auto rule : m_rules) {
		std::vector<TypeId> fn_params;
		for (auto param : rule.param_list) {
			TypeId param_tid = param.type->Verify(ctx).GetResultTypeId();
			fn_params.push_back(param_tid);
		}
		TypeId fn_return_tid = rule.return_type->Verify(ctx).GetResultTypeId();
		TypeId fn_tid		 = g_typemgr.GetOrAddTypeFn(fn_params, fn_return_tid);

		concrete_rules.push_back(TypeInfoRestriction::Rule{.fn_name = rule.fnname, .fn_tid = fn_tid});
	}

	ctx.GetCurStack()->LeaveBlock();

	// 生成实例化后的restriction的唯一名字
	std::string uniq_restriction_name = m_name + "[";
	for (size_t i = 0; i < concrete_params.size(); i++) {
		uniq_restriction_name += GET_TYPENAME(concrete_params.at(i));
		if (i + 1 < concrete_params.size()) {
			uniq_restriction_name += ",";
		}
	}
	uniq_restriction_name += "]";

	Variable* restriction_v = ctx.GetCurStack()->GetVariableOrNull(uniq_restriction_name);
	if (restriction_v != nullptr) {
		log_debug("generic restriction[%s] instance_name=%s already instantiated with typeid=%d", m_name.c_str(), uniq_restriction_name.c_str(), restriction_v->GetTypeId());
		return restriction_v->GetTypeId();
	}

	TypeInfoRestriction* restriction_ti	 = new TypeInfoRestriction(uniq_restriction_name, concrete_rules);
	TypeId				 restriction_tid = g_typemgr.GetOrAddTypeRestriction(restriction_ti);
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(uniq_restriction_name, new Variable(restriction_tid));
	log_debug("instantiate restriction[%s]: name=%s typeid=%d", m_name.c_str(), uniq_restriction_name.c_str(), restriction_tid);
	return restriction_tid;
}
