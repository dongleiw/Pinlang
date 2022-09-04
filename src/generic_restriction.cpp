#include "generic_restriction.h"
#include "define.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_restriction.h"
#include <memory>

TypeId get_tid_by_generics_param_name(const std::vector<GenericRestriction::GenericParam>& generic_params, std::string generics_param_name) {
	for (auto iter : generic_params) {
		if (iter.name == generics_param_name) {
			return iter.real_tid;
		}
	}
	return TYPE_ID_NONE;
}

TypeId GenericRestriction::Instantiate(std::map<std::string, TypeId> params) const {
	std::vector<GenericParam> instantiate_param = m_generic_params;

	// 将传入参数赋值
	for (auto iter : instantiate_param) {
		auto found = params.find(iter.name);
		if (found == params.end()) {
		} else {
			iter.real_tid = found->second;
		}
	}

	// 推导缺省的参数
	for (auto iter : instantiate_param) {
		if (iter.real_tid == TYPE_ID_NONE) {
			if (iter.default_value_name.empty()) {
				panicf("generics param[%s] is missed", iter.name.c_str());
			} else {
				TypeId refered_tid = get_tid_by_generics_param_name(instantiate_param, iter.default_value_name);
				if (refered_tid == TYPE_ID_NONE) {
					panicf("generics param[%s] is missed", iter.name.c_str());
				}
				iter.real_tid = refered_tid;
			}
		}
	}

	// 实例化restriction中的方法
	std::vector<TypeInfoRestriction::Rule> instantiate_rules;
	for (auto rule_iter : m_generic_rules) {
		std::vector<Parameter> fn_params;
		for (auto fn_param_iter : rule_iter.function.params_type) {
			TypeId fn_param_real_tid = get_tid_by_generics_param_name(instantiate_param, fn_param_iter);
			if (fn_param_real_tid == TYPE_ID_NONE) {
				panicf("generics param[%s] is missed", fn_param_iter.c_str());
			}
			fn_params.push_back(Parameter{.arg_tid = fn_param_real_tid});
		}

		TypeId fn_return_tid = get_tid_by_generics_param_name(instantiate_param, rule_iter.function.return_type);
		if (fn_return_tid == TYPE_ID_NONE) {
			panicf("generics param[%s] is missed", rule_iter.function.return_type.c_str());
		}

		TypeId fn_tid = g_typemgr.GetOrAddTypeFn(fn_params, fn_return_tid);

		instantiate_rules.push_back(TypeInfoRestriction::Rule{
			.fn_name = rule_iter.fn_name,
			.fn_tid	 = fn_tid,
		});
	}

	// 生成实例化后的restriction的唯一名字
	std::string uniq_restriction_name = m_name + "<";
	for (size_t i = 0; i < instantiate_param.size(); i++) {
		uniq_restriction_name += GET_TYPENAME(instantiate_param.at(i).real_tid);
		if (i + 1 < instantiate_param.size()) {
			uniq_restriction_name += ",";
		}
	}
	uniq_restriction_name += ">";

	// 加入typemgr
	TypeId restriction_instance_tid = g_typemgr.GetOrAddTypeRestriction(uniq_restriction_name, instantiate_rules);
	log_debug("instantiate restriction[%s]: name=%s typeid=%d", m_name.c_str(), uniq_restriction_name.c_str(), restriction_instance_tid);
	return restriction_instance_tid;
}
