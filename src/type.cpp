#include "type.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "function_obj.h"
#include "log.h"
#include "type_constraint.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "verify_context.h"
#include <cassert>

bool TypeInfo::MatchConstraint(ConstraintInstance constraint_instance) const {
	for (auto constraint : m_constraints) {
		if (constraint.constraint_instance.constraint_instance_name == constraint_instance.constraint_instance_name) {
			assert(constraint.constraint_instance.constraint_name == constraint_instance.constraint_name);
			return true;
		}
	}
	return false;
}
void TypeInfo::AddConstraint(ConstraintInstance constraint_instance, std::vector<AstNodeComplexFnDef*> methods) {
	for (auto& constraint : m_constraints) {
		if (constraint.constraint_instance.constraint_instance_name == constraint_instance.constraint_instance_name) {
			panicf("constraint already exists");
		}
	}
	Constraint constraint;
	constraint.constraint_instance = constraint_instance;
	for (auto iter : methods) {
		iter->SetObjTypeId(GetTypeId());
		constraint.methods.push_back(Method{
			.method_name = iter->GetName(),
			.method_node = iter,
		});
	}
	m_constraints.push_back(constraint);
}
std::vector<std::string> TypeInfo::GetConstraintMethod(VerifyContext& ctx, std::string constraint_name, std::string method_name, std::vector<TypeId> method_params_tid, TypeId return_tid) {
	std::vector<std::string> method_indexs;
	for (auto& constraint : m_constraints) {
		if (constraint.constraint_instance.constraint_instance_name.empty()) {
			if (constraint_name.empty()) {
				for (auto& m : constraint.methods) {
					if (m.method_name == method_name) {
						std::string fn_id = m.method_node->Instantiate_param_return(ctx, method_params_tid, return_tid);
						if (!fn_id.empty()) {
							method_indexs.push_back(fn_id);
						}
					}
				}
			}
		} else {
			if (constraint.constraint_instance.constraint_name == constraint_name) {
				for (size_t i = 0; i < constraint.methods.size(); i++) {
					const Method& method = constraint.methods.at(i);
					if (method.method_name == method_name) {
						std::string fn_id = method.method_node->Instantiate_param_return(ctx, method_params_tid, return_tid);
						if (!fn_id.empty()) {
							method_indexs.push_back(fn_id);
						}
					}
				}
			}
		}
	}
	return method_indexs;
}
std::vector<std::string> TypeInfo::GetConstraintMethod(VerifyContext& ctx, std::string method_name, std::vector<TypeId> method_params_tid, TypeId return_tid) {
	std::vector<std::string> method_indexs;
	for (auto& constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			const Method& method = constraint.methods.at(i);
			if (method.method_name == method_name) {
				std::string fn_id = method.method_node->Instantiate_param_return(ctx, method_params_tid, return_tid);
				if (!fn_id.empty()) {
					method_indexs.push_back(fn_id);
				}
			}
		}
	}
	return method_indexs;
}
std::vector<std::string> TypeInfo::GetConstraintMethod(VerifyContext& ctx, std::string constraint_name, std::string method_name, TypeId tid) {
	TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(tid));
	return GetConstraintMethod(ctx, constraint_name, method_name, ti->GetParmsTid(), ti->GetReturnTypeId());
}
std::vector<std::string> TypeInfo::GetConstraintMethod(VerifyContext& ctx, std::string method_name, TypeId tid) {
	TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(tid));
	return GetConstraintMethod(ctx, method_name, ti->GetParmsTid(), ti->GetReturnTypeId());
}
std::vector<std::string> TypeInfo::GetConstraintMethod(VerifyContext& ctx, std::string constraint_name, std::string method_name) {
	std::vector<std::string> method_indexs;
	for (auto& constraint : m_constraints) {
		if (constraint.constraint_instance.constraint_instance_name.empty()) {
			if (constraint_name.empty()) {
				for (auto& c : m_constraints) {
					for (auto& m : c.methods) {
						std::string fn_id = m.method_node->Instantiate(ctx);
						if (!fn_id.empty()) {
							method_indexs.push_back(fn_id);
						}
					}
				}
			}
		} else {
			if (constraint.constraint_instance.constraint_name == constraint_name) {
				for (size_t i = 0; i < constraint.methods.size(); i++) {
					const Method& method = constraint.methods.at(i);
					if (method.method_name == method_name) {
						std::string fn_id = method.method_node->Instantiate(ctx);
						if (!fn_id.empty()) {
							method_indexs.push_back(fn_id);
						}
					}
				}
			}
		}
	}
	return method_indexs;
}
std::vector<std::string> TypeInfo::GetConstraintMethod(VerifyContext& ctx, std::string method_name) {
	std::vector<std::string> method_indexs;
	for (auto& constraint : m_constraints) {
		for (size_t i = 0; i < constraint.methods.size(); i++) {
			const Method& method = constraint.methods.at(i);
			if (method.method_name == method_name) {
				std::string fn_id = method.method_node->Instantiate(ctx);
				if (!fn_id.empty()) {
					method_indexs.push_back(fn_id);
				}
			}
		}
	}
	return method_indexs;
}
std::vector<std::string> TypeInfo::GetConstructor(VerifyContext& ctx, std::string method_name, std::vector<TypeId> params_tid) {
	std::vector<std::string> method_indexs;
	for (auto& constraint : m_constraints) {
		if (!constraint.constraint_instance.constraint_name.empty()) {
			continue;
		}
		for (auto& c : m_constraints) {
			for (auto& m : c.methods) {
				if (m.method_node->GetFnAttr() & FnAttr::FN_ATTR_CONSTRUCTOR && m.method_node->GetName() == method_name) {
					std::string fn_id = m.method_node->Instantiate_param_return(ctx, params_tid, TYPE_ID_NONE);
					if (!fn_id.empty()) {
						method_indexs.push_back(fn_id);
					}
				}
			}
		}
	}
	return method_indexs;
}
std::vector<std::string> TypeInfo::GetConstructor(VerifyContext& ctx, std::string method_name) {
	std::vector<std::string> method_indexs;
	for (auto& constraint : m_constraints) {
		if (!constraint.constraint_instance.constraint_name.empty()) {
			continue;
		}
		for (auto& c : m_constraints) {
			for (auto& m : c.methods) {
				if (m.method_node->GetFnAttr() & FnAttr::FN_ATTR_CONSTRUCTOR && m.method_node->GetName() == method_name) {
					std::string fn_id = m.method_node->Instantiate(ctx);
					if (!fn_id.empty()) {
						method_indexs.push_back(fn_id);
					}
				}
			}
		}
	}
	return method_indexs;
}
bool TypeInfo::IsSimpleConstrcutor(std::string method_name) const {
	std::vector<std::string> method_indexs;
	for (auto& constraint : m_constraints) {
		if (!constraint.constraint_instance.constraint_name.empty()) {
			continue;
		}
		for (auto& c : m_constraints) {
			for (auto& m : c.methods) {
				if (m.method_node->GetFnAttr() & FnAttr::FN_ATTR_CONSTRUCTOR && m.method_node->GetName() == method_name) {
					if (m.method_node->IsSimpleFn()) {
						return true;
					}
				}
			}
		}
	}
	return false;
}
bool TypeInfo::IsSimpleMethod(std::string method_name) const {
	std::vector<std::string> method_indexs;
	for (auto& constraint : m_constraints) {
		for (auto& c : m_constraints) {
			for (auto& m : c.methods) {
				if (m.method_node->GetName() == method_name) {
					if (m.method_node->IsSimpleFn()) {
						return true;
					}
				}
			}
		}
	}
	return false;
}
bool TypeInfo::HasConstructor() const {
	for (auto& constraint : m_constraints) {
		if (!constraint.constraint_instance.constraint_name.empty()) {
			continue;
		}
		for (auto& c : m_constraints) {
			for (auto& m : c.methods) {
				if (m.method_node->GetFnAttr() & FnAttr::FN_ATTR_CONSTRUCTOR) {
					return true;
				}
			}
		}
	}
	return false;
}
llvm::Type* TypeInfo::GetLLVMIRType(CompileContext& cctx) {
	panicf("not implemented");
	return nullptr;
}
void TypeInfo::ConstructFields(CompileContext& cctx, llvm::Value* obj) {
}
void TypeInfo::ConstructDefault(CompileContext& cctx, llvm::Value* obj) {
	panicf("not implemented");
}
void TypeInfo::SetTypeId(TypeId tid) {
	m_typeid = tid;
	if (IsClass()) {
		m_name = m_original_name + "#" + to_str(m_typeid);
	} else {
		m_name = m_original_name;
	}
}
