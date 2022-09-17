#include "type.h"
#include "function.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"

int TypeInfo::GetMethodIdx(std::string method_name, std::vector<TypeId> args_tid) const {
	int				 i = 0;
	std::vector<int> match_idx_list;
	for (auto iter : m_methods) {
		if (iter.name == method_name) {
			TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(iter.f->GetTypeId()));
			if (ti->IsArgsTypeEqual(args_tid)) {
				match_idx_list.push_back(i);
			}
		}
		i++;
	}
	if (match_idx_list.empty()) {
		return -1;
	} else if (match_idx_list.size() > 1) {
		panicf("multiple candidate of method[%s] with args[%s]", method_name.c_str(), g_typemgr.GetTypeName(args_tid).c_str());
	} else {
		return match_idx_list.at(0);
	}
}
int TypeInfo::GetMethodIdx(TypeId constraint_tid, std::string method_name, std::vector<TypeId> args_tid) const {
	int i = 0;
	for (auto iter : m_methods) {
		if (iter.name == method_name && iter.constraint_tid == constraint_tid) {
			TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(iter.f->GetTypeId()));
			if (ti->IsArgsTypeEqual(args_tid)) {
				return i;
			}
		}
		i++;
	}
	return -1;
}
int TypeInfo::GetMethodIdx(TypeId constraint_tid, std::string method_name, TypeId tid) const {
	int i = 0;
	for (auto iter : m_methods) {
		if (iter.name == method_name && iter.constraint_tid == constraint_tid) {
			if (iter.f->GetTypeId() == tid)
				return i;
		}
		i++;
	}
	return -1;
}
Function* TypeInfo::GetMethodByIdx(int idx) {
	return m_methods.at(idx).f;
}
void TypeInfo::AddMethod(TypeId constraint_tid, std::string method_name, Function* function) {
	if (has_duplicate_method(constraint_tid, method_name, function->GetTypeId())) {
		panicf("duplicate method[%s] constraint[%d]", method_name.c_str(), constraint_tid);
	}

	m_methods.push_back(Method{
		.constraint_tid = constraint_tid,
		.name			 = method_name,
		.f				 = function,
	});
}
bool TypeInfo::has_duplicate_method(TypeId constraint_tid, std::string method_name, TypeId new_tid) const {
	TypeInfoFn* new_tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(new_tid));
	for (auto iter : m_methods) {
		if (iter.name == method_name && iter.constraint_tid == constraint_tid) {
			TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(iter.f->GetTypeId()));
			if (tifn->IsArgsTypeEqual(*new_tifn)) {
				return true;
			}
		}
	}
	return false;
}
void TypeInfo::AddBuiltinMethod(TypeId constraint_tid, std::string method_name, std::vector<TypeId> params_tid, TypeId ret_tid, BuiltinFnCallback cb) {
	TypeId	  tid = g_typemgr.GetOrAddTypeFn(params_tid, ret_tid);
	Function* f	  = new Function(tid, cb);
	AddMethod(constraint_tid, method_name, f);
}
bool TypeInfo::MatchConstraint(TypeId tid) const {
	// TODO 这个判断方式有点投机取巧
	for (auto iter : m_methods) {
		if (iter.constraint_tid == tid) {
			return true;
		}
	}
	return false;
}
void TypeInfo::AddConstraint(TypeId constraint_tid, std::map<std::string, Function*> methods) {
	for (auto iter : methods) {
		AddMethod(constraint_tid, iter.first, iter.second);
	}
}
