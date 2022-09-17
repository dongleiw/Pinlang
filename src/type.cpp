#include "type.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include <cassert>

MethodIndex TypeInfo::GetMethodIdx(std::string method_name, std::vector<TypeId> args_tid) const {
	TypeId			 constraint_tid = TYPE_ID_NONE;
	std::vector<int> match_idx_list;
	for (auto& constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			if (constraint.methods.at(i).method_name == method_name) {
				TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(constraint.methods.at(i).fn->GetTypeId()));
				if (ti->IsArgsTypeEqual(args_tid)) {
					match_idx_list.push_back(i);
					constraint_tid = constraint.constraint_tid;
				}
			}
		}
	}
	if (match_idx_list.empty()) {
		return MethodIndex();
	} else if (match_idx_list.size() > 1) {
		panicf("multiple candidate of method[%s] with args[%s]", method_name.c_str(), g_typemgr.GetTypeName(args_tid).c_str());
	} else {
		return MethodIndex(constraint_tid, match_idx_list.at(0));
	}
}
MethodIndex TypeInfo::GetMethodIdx(std::string method_name, TypeId tid) const {
	TypeId			 constraint_tid = TYPE_ID_NONE;
	std::vector<int> match_idx_list;
	for (auto& constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			if (constraint.methods.at(i).method_name == method_name && constraint.methods.at(i).fn->GetTypeId() == tid) {
				match_idx_list.push_back(i);
				constraint_tid = constraint.constraint_tid;
			}
		}
	}
	if (match_idx_list.empty()) {
		return MethodIndex();
	} else if (match_idx_list.size() > 1) {
		panicf("multiple candidate of method[%s] of type[%d:%s]", method_name.c_str(), tid, GET_TYPENAME_C(tid));
	} else {
		return MethodIndex(constraint_tid, match_idx_list.at(0));
	}
}
MethodIndex TypeInfo::GetMethodIdx(TypeId constraint_tid, std::string method_name, std::vector<TypeId> args_tid) const {
	std::vector<int> match_idx_list;
	for (auto& constraint : m_constraints) {
		if (constraint.constraint_tid != constraint_tid)
			continue;
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			if (constraint.methods.at(i).method_name == method_name) {
				TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(constraint.methods.at(i).fn->GetTypeId()));
				if (ti->IsArgsTypeEqual(args_tid)) {
					match_idx_list.push_back(i);
				}
			}
		}
	}
	if (match_idx_list.empty()) {
		return MethodIndex();
	} else if (match_idx_list.size() > 1) {
		panicf("multiple candidate of method[%s] with args[%s]", method_name.c_str(), g_typemgr.GetTypeName(args_tid).c_str());
	} else {
		return MethodIndex(constraint_tid, match_idx_list.at(0));
	}
}
MethodIndex TypeInfo::GetMethodIdx(TypeId constraint_tid, std::string method_name, TypeId tid) const {
	std::vector<int> match_idx_list;
	for (auto& constraint : m_constraints) {
		if (constraint.constraint_tid != constraint_tid)
			continue;
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			if (constraint.methods.at(i).method_name == method_name && constraint.methods.at(i).fn->GetTypeId() == tid) {
				match_idx_list.push_back(i);
			}
		}
	}
	if (match_idx_list.empty()) {
		return MethodIndex();
	} else if (match_idx_list.size() > 1) {
		panicf("multiple candidate of method[%s] of type[%d:%s]", method_name.c_str(), tid, GET_TYPENAME_C(tid));
	} else {
		return MethodIndex(constraint_tid, match_idx_list.at(0));
	}
}
MethodIndex TypeInfo::GetMethodIdx(std::string method_name) const {
	TypeId			 constraint_tid = TYPE_ID_NONE;
	std::vector<int> match_idx_list;
	for (auto& constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			if (constraint.methods.at(i).method_name == method_name) {
				match_idx_list.push_back(i);
				constraint_tid = constraint.constraint_tid;
			}
		}
	}
	if (match_idx_list.empty()) {
		return MethodIndex();
	} else if (match_idx_list.size() > 1) {
		panicf("multiple candidate of method[%s]", method_name.c_str());
	} else {
		return MethodIndex(constraint_tid, match_idx_list.at(0));
	}
}
Function* TypeInfo::GetMethodByIdx(MethodIndex method_idx) {
	assert(method_idx.IsValid());
	for (auto constraint : m_constraints) {
		if (constraint.constraint_tid != method_idx.constraint_tid)
			continue;
		return constraint.methods.at(method_idx.method_idx).fn;
	}
	panicf("invalid method idx: constraint_tid[%d] method_idx[%d]", method_idx.constraint_tid, method_idx.method_idx);
	return nullptr;
}
bool TypeInfo::MatchConstraint(TypeId tid) const {
	for (auto constraint : m_constraints) {
		if (constraint.constraint_tid == tid)
			return true;
	}
	return false;
}
void TypeInfo::AddConstraint(TypeId constraint_tid, std::map<std::string, Function*> methods) {
	for(auto& constraint:m_constraints){
		if(constraint.constraint_tid==constraint_tid){
			panicf("constraint already exists");
		}
	}
	Constraint constraint;
	constraint.constraint_tid = constraint_tid;
	for (auto iter : methods) {
		constraint.methods.push_back(Method{.method_name=iter.first, .fn=iter.second});
	}
	m_constraints.push_back(constraint);
}
bool TypeInfo::HasField(std::string attr_name) const {
	// TODO 没实现
	return false;
}
