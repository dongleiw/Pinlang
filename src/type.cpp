#include "type.h"
#include "function.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"

bool TypeInfo::HasMethod(std::string method_name) const {
	return m_methods.find(method_name) != m_methods.end();
}
void TypeInfo::AddMethod(std::string method_name, Function* function) {
	if (HasMethod(method_name)) {
		panicf("duplicate method[%s]", method_name.c_str());
	}
	m_methods[method_name] = function;
}
Function* TypeInfo::GetMethodOrNilByName(std::string method_name) {
	auto iter = m_methods.find(method_name);
	if (iter == m_methods.end()) {
		return nullptr;
	} else {
		return iter->second;
	}
}
Function* TypeInfo::GetMethodByName(std::string method_name) {
	auto iter = m_methods.find(method_name);
	if (iter == m_methods.end()) {
		panicf("type[%d:%s] doesn't have method[%s]", GetTypeId(), GetName().c_str(), method_name.c_str());
		return nullptr;
	} else {
		return iter->second;
	}
}
void TypeInfo::AddBuiltinMethod(std::string method_name, std::vector<TypeId> params_tid, TypeId ret_tid, BuiltinFnCallback cb) {
	std::string			   uniq_method_name = method_name + "_" + TypeInfoFn::GetUniqFnNameSuffix(params_tid);
	std::vector<Parameter> params;
	for (auto iter : params_tid) {
		params.push_back({.arg_tid = iter});
	}
	TypeId	  tid = g_typemgr.GetOrAddTypeFn(params, ret_tid);
	Function* f	  = new Function(tid, cb);
	AddMethod(uniq_method_name, f);
}
