#include "type.h"
#include "function.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include <cassert>

int TypeInfo::GetMethodIdx(std::string method_name, std::vector<TypeId> args_tid) const {
	int				 i = 0;
	std::vector<int> match_idx_list;
	for (auto attr : m_attrs) {
		if (attr.is_field)
			continue;
		if (attr.method.method_name == method_name) {
			TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(attr.method.fn->GetTypeId()));
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
int TypeInfo::GetMethodIdx(std::string method_name, TypeId tid) const {
	int				 i = 0;
	std::vector<int> match_idx_list;
	for (auto attr : m_attrs) {
		if (attr.is_field)
			continue;
		if (attr.method.method_name == method_name && attr.method.fn->GetTypeId() == tid) {
			match_idx_list.push_back(i);
		}
		i++;
	}
	if (match_idx_list.empty()) {
		return -1;
	} else if (match_idx_list.size() > 1) {
		panicf("multiple candidate of method[%s] of type[%d:%s]", method_name.c_str(), tid, GET_TYPENAME_C(tid));
	} else {
		return match_idx_list.at(0);
	}
}
int TypeInfo::GetMethodIdx(TypeId constraint_tid, std::string method_name, std::vector<TypeId> args_tid) const {
	int i = 0;
	for (auto attr : m_attrs) {
		if (attr.is_field)
			continue;
		if (attr.method.method_name == method_name && attr.method.constraint_tid == constraint_tid) {
			TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(attr.method.fn->GetTypeId()));
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
	for (auto attr : m_attrs) {
		if (attr.is_field)
			continue;
		if (attr.method.method_name == method_name && attr.method.constraint_tid == constraint_tid) {
			if (attr.method.fn->GetTypeId() == tid)
				return i;
		}
		i++;
	}
	return -1;
}
Function* TypeInfo::GetMethodByIdx(int idx) {
	assert(m_attrs.at(idx).is_field == false);
	return m_attrs.at(idx).method.fn;
}
void TypeInfo::AddMethod(TypeId constraint_tid, std::string method_name, Function* function) {
	if (has_duplicate_method(constraint_tid, method_name, function->GetTypeId())) {
		panicf("duplicate method[%s] constraint[%d]", method_name.c_str(), constraint_tid);
	}

	Attr attr;
	attr.InitMethod(constraint_tid, method_name, function);
	m_attrs.push_back(attr);
}
bool TypeInfo::has_duplicate_method(TypeId constraint_tid, std::string method_name, TypeId new_tid) const {
	TypeInfoFn* new_tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(new_tid));
	for (auto attr : m_attrs) {
		if (attr.is_field)
			continue;
		if (attr.method.constraint_tid == constraint_tid && attr.method.method_name == method_name) {
			TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(attr.method.fn->GetTypeId()));
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
	for (auto attr : m_attrs) {
		if (!attr.is_field && attr.method.constraint_tid == tid) {
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
bool TypeInfo::HasField(std::string attr_name) const {
	for (auto attr : m_attrs) {
		if (attr.is_field && attr.field.field_name == attr_name) {
			return true;
		}
	}
	return false;
}
bool TypeInfo::HasSingleMethod(std::string attr_name, size_t& out_attr_idx) const {
	int	   count	= 0;
	size_t attr_idx = 0;
	for (size_t i = 0; i < m_attrs.size(); i++) {
		if (!m_attrs.at(i).is_field && m_attrs.at(i).method.method_name == attr_name) {
			count++;
			attr_idx = i;
		}
	}
	if (count == 1) {
		out_attr_idx = attr_idx;
		return true;
	}
	return false;
}
Attr TypeInfo::GetAttr(int attr_idx) {
	return m_attrs.at(attr_idx);
}
