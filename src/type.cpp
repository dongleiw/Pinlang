#include "type.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "function.h"
#include "function_obj.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "verify_context.h"
#include <cassert>

MethodIndex TypeInfo::GetMethodIdx(std::string method_name) const {
	TypeId			 constraint_tid = TYPE_ID_NONE;
	std::vector<int> match_idx_list;
	for (auto& constraint : m_constraints) {
		for (size_t i = 0; i < constraint.concrete_methods.size(); i++) {
			if (constraint.concrete_methods.at(i).method_name == method_name) {
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
		return constraint.concrete_methods.at(method_idx.method_idx).fn;
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
void TypeInfo::AddConstraint(TypeId constraint_tid, std::vector<AstNodeComplexFnDef*> methods) {
	for (auto& constraint : m_constraints) {
		if (constraint.constraint_tid == constraint_tid) {
			panicf("constraint already exists");
		}
	}
	Constraint constraint;
	constraint.constraint_tid = constraint_tid;
	for (auto iter : methods) {
		iter->SetObjTypeId(GetTypeId());
		constraint.methods.push_back(Method{
			.method_name = iter->GetName(),
			.method_node = iter,
		});
	}
	m_constraints.push_back(constraint);
}
bool TypeInfo::HasField(std::string field_name) const {
	for (auto iter : m_field_list) {
		if (iter.name == field_name) {
			return true;
		}
	}
	return false;
}
void TypeInfo::AddField(std::string field_name, TypeId tid) {
	assert(!HasField(field_name));
	m_field_list.push_back(Field{
		.name = field_name,
		.tid  = tid,
	});
}
TypeId TypeInfo::GetFieldType(std::string field_name) const {
	for (auto iter : m_field_list) {
		if (iter.name == field_name) {
			return iter.tid;
		}
	}
	panicf("bug");
}
MethodIndex TypeInfo::GetConcreteMethod(VerifyContext& ctx, std::string method_name, std::vector<TypeId> args_tid, TypeId return_tid) {
	int method_count = 0;
	for (auto constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			const Method& method = constraint.methods.at(i);
			if (method.method_name == method_name) {
				method_count++;
			}
		}
	}
	if (method_count > 1) {
		// 多个约束有同名方法. 目前处理不了
		panicf("same method name[%s] in multiple constraint", method_name.c_str());
	}

	for (auto& constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			const Method& method = constraint.methods.at(i);
			if (method.method_name == method_name) {
				AstNodeComplexFnDef::Instance method_instance = method.method_node->Instantiate_param_return(ctx, args_tid, return_tid);

				MethodIndex method_index = constraint.AddConcreteMethod(method_instance.instance_name, method_instance.fnobj.GetFunction());
				return method_index;
			}
		}
	}
	return MethodIndex();
}
MethodIndex TypeInfo::GetConcreteMethod(VerifyContext& ctx, std::string method_name, TypeId tid) {
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(tid));
	return GetConcreteMethod(ctx, method_name, tifn->GetParmsTid(), tifn->GetReturnTypeId());
}
MethodIndex TypeInfo::GetConcreteMethod(VerifyContext& ctx, std::string method_name) {
	int method_count = 0;
	for (auto constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			const Method& method = constraint.methods.at(i);
			if (method.method_name == method_name) {
				method_count++;
			}
		}
	}
	if (method_count > 1) {
		// 多个约束有同名方法. 目前处理不了
		panicf("same method name[%s] in multiple constraint", method_name.c_str());
	}

	for (auto& constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			const Method& method = constraint.methods.at(i);
			if (method.method_name == method_name) {
				AstNodeComplexFnDef::Instance method_instance = method.method_node->Instantiate(ctx);

				MethodIndex method_index = constraint.AddConcreteMethod(method_instance.instance_name, method_instance.fnobj.GetFunction());
				return method_index;
			}
		}
	}
	return MethodIndex();
}
MethodIndex TypeInfo::Constraint::AddConcreteMethod(std::string method_name, Function* fn) {
	for (size_t i = 0; i < concrete_methods.size(); i++) {
		if (concrete_methods.at(i).method_name == method_name) {
			return MethodIndex(constraint_tid, i);
		}
	}
	concrete_methods.push_back(MethodInstance{
		.method_name = method_name,
		.fn			 = fn,
	});

	return MethodIndex(constraint_tid, concrete_methods.size() - 1);
}
