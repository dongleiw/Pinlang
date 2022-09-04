#include "type_fn.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include <functional>

TypeInfoFn::TypeInfoFn(std::vector<TypeId> params, TypeId return_tid) {
	m_params		   = params;
	m_return_tid	   = return_tid;
	set_name();
	//set_uniq_fn_name_suffix();
	m_typegroup_id = TYPE_GROUP_ID_FUNCTION;
}

std::string TypeInfoFn::GetUniqFnName(std::string fnname, std::vector<TypeId> concrete_generic_params, std::vector<TypeId> params_tid) {
	// 生成实例的唯一名字. 形式为: fn-name[泛型参数的实际类型id...](各参数类型id...)
	// 这里使用泛型参数的typeid而不是typename, 是因为typename可能会重复
	std::string s = fnname + "[";
	char		buf[8];
	for (size_t i = 0; i < concrete_generic_params.size(); i++) {
		snprintf(buf, sizeof(buf), "%d", concrete_generic_params.at(i));
		s += buf;
		if (i + 1 != concrete_generic_params.size()) {
			s += ",";
		}
	}
	s += "](";
	for (size_t i = 0; i < params_tid.size(); i++) {
		snprintf(buf, sizeof(buf), "%d", params_tid.at(i));
		s += buf;
		if (i + 1 != params_tid.size()) {
			s += ",";
		}
	}
	s += ")";
	return s;
}
std::string TypeInfoFn::GetUniqFnName(std::string fnname, std::vector<TypeId> params_tid) {
	return GetUniqFnName(fnname, std::vector<TypeId>(), params_tid);
}
void TypeInfoFn::set_name() {
	m_uniq_fnname_suffix = GetUniqFnName("", m_params);
	m_name				 = "fn" + m_uniq_fnname_suffix + GET_TYPENAME(m_return_tid);
}
bool TypeInfoFn::VerifyArgsType(std::vector<TypeId> args_type) {
	if (args_type.size() != m_params.size()) {
		return false;
	}

	for (size_t i = 0; i < m_params.size(); i++) {
		TypeId expect_tid = m_params.at(i);
		TypeId give_tid	  = args_type.at(i);
		if (expect_tid != give_tid) {
			log_error("type of arg %lu not match: expect[%s] give[%s]", i, GET_TYPENAME_C(expect_tid), GET_TYPENAME_C(give_tid));
			return false;
		}
	}
	return true;
}
bool TypeInfoFn::IsArgsTypeEqual(const TypeInfoFn& another) const {
	if (m_params.size() != another.m_params.size()) {
		return false;
	}
	for (size_t i = 0; i < m_params.size(); i++) {
		if (m_params.at(i) != another.m_params.at(i)) {
			return false;
		}
	}
	return true;
}
bool TypeInfoFn::IsArgsTypeEqual(std::vector<TypeId> args_tid) const {
	if (m_params.size() != args_tid.size()) {
		return false;
	}
	for (size_t i = 0; i < m_params.size(); i++) {
		if (m_params.at(i) != args_tid.at(i)) {
			return false;
		}
	}
	return true;
}
std::vector<TypeId> TypeInfoFn::GetParmsTid() const {
	std::vector<TypeId> params_tid;
	for (auto iter : m_params) {
		params_tid.push_back(iter);
	}
	return params_tid;
}
