#include "astnode_constraint.h"
#include "astnode_type.h"
#include "define.h"
#include "type.h"
#include "type_constraint.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "variable_table.h"
#include "verify_context.h"
#include <cassert>

AstNodeConstraint::AstNodeConstraint(std::string name, std::vector<std::string> generic_params, std::vector<ParserFnDeclare> rules) {
	m_name			 = name;
	m_generic_params = generic_params;
	m_rules			 = rules;
}
VerifyContextResult AstNodeConstraint::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify constraint");

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_name, new Variable(this));
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeConstraint::Execute(ExecuteContext& ctx) {
	return nullptr;
}
TypeId AstNodeConstraint::Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params) {
	//assert(HasGenericParam());
	if (concrete_params.size() != m_generic_params.size()) {
		panicf("wrong number of generic_params");
	}

	// 泛型名映射到实际类型id
	VariableTable* vt = new VariableTable();
	for (size_t i = 0; i < m_generic_params.size(); i++) {
		vt->AddVariable(m_generic_params.at(i), Variable::CreateTypeVariable(concrete_params.at(i)));
	}

	// 添加一个临时vt, 避免污染现有vt
	ctx.GetCurStack()->EnterBlock(vt);

	// 遍历所有rule模板, 生成实际类型
	std::vector<TypeInfoConstraint::Rule> concrete_rules;
	for (auto rule : m_rules) {
		std::vector<TypeId> fn_params;
		for (auto param : rule.param_list) {
			TypeId param_tid = param.type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
			fn_params.push_back(param_tid);
		}
		TypeId fn_return_tid = TYPE_ID_NONE;
		if (rule.return_type != nullptr) {
			fn_return_tid = rule.return_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		}
		TypeId fn_tid = g_typemgr.GetOrAddTypeFn(fn_params, fn_return_tid);

		concrete_rules.push_back(TypeInfoConstraint::Rule{.fn_name = rule.fnname, .fn_tid = fn_tid});
	}

	ctx.GetCurStack()->LeaveBlock();

	// 生成实例化后的constraint的唯一名字
	std::string uniq_constraint_name = m_name + "[";
	for (size_t i = 0; i < concrete_params.size(); i++) {
		uniq_constraint_name += GET_TYPENAME(concrete_params.at(i));
		if (i + 1 < concrete_params.size()) {
			uniq_constraint_name += ",";
		}
	}
	uniq_constraint_name += "]";

	auto found = m_instances.find(uniq_constraint_name);
	if (found == m_instances.end()) {
		TypeInfoConstraint* constraint_ti  = new TypeInfoConstraint(uniq_constraint_name, concrete_rules);
		TypeId				constraint_tid = g_typemgr.GetOrAddTypeConstraint(constraint_ti);
		m_instances[uniq_constraint_name] = constraint_tid;
		log_debug("instantiate constraint[%s]: name=%s typeid=%d", m_name.c_str(), uniq_constraint_name.c_str(), constraint_tid);
		return constraint_tid;
	} else {
		log_debug("generic constraint[%s] instance_name=%s already instantiated with typeid=%d", m_name.c_str(), uniq_constraint_name.c_str(), found->second);
		return found->second;
	}
	//Variable* constraint_v = ctx.GetCurStack()->GetVariableOrNull(uniq_constraint_name);
	//if (constraint_v != nullptr) {
	//	return constraint_v->GetTypeId();
	//}

	//TypeInfoConstraint* constraint_ti  = new TypeInfoConstraint(uniq_constraint_name, concrete_rules);
	//TypeId				constraint_tid = g_typemgr.GetOrAddTypeConstraint(constraint_ti);
	//ctx.GetCurStack()->GetCurVariableTable()->AddVariable(uniq_constraint_name, new Variable(constraint_tid));
	//return constraint_tid;
}
AstNodeConstraint* AstNodeConstraint::DeepCloneT() {
	AstNodeConstraint* newone = new AstNodeConstraint();

	newone->m_name			 = m_name;
	newone->m_generic_params = m_generic_params;
	for (auto iter : m_rules) {
		newone->m_rules.push_back(iter.DeepClone());
	}

	return newone;
}
