#include "astnode_generic_instantiate.h"
#include "astnode_type.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <cassert>

AstNodeGenericInstantiate::AstNodeGenericInstantiate(std::string generic_name, std::vector<AstNodeType*> type_list) {
	m_generic_name = generic_name;
	m_type_list	   = type_list;
}
VerifyContextResult AstNodeGenericInstantiate::Verify(VerifyContext& ctx, VerifyContextParam vr_param) {
	log_debug("begin to verify generic instantiate");

	// 得到泛参的实际类型
	std::vector<TypeId> gparams_tid;
	for (auto iter : m_type_list) {
		gparams_tid.push_back(iter->Verify(ctx, VerifyContextParam()).GetResultTypeId());
	}

	VerifyContextResult vr(m_result_typeid);
	return vr;
}
Variable* AstNodeGenericInstantiate::Execute(ExecuteContext& ctx) {
	return ctx.GetCurStack()->GetVariable(m_instance_name);
}
AstNodeGenericInstantiate* AstNodeGenericInstantiate::DeepCloneT() {
	AstNodeGenericInstantiate* newone = new AstNodeGenericInstantiate();

	newone->m_generic_name = m_generic_name;
	for (auto iter : m_type_list) {
		newone->m_type_list.push_back(iter->DeepCloneT());
	}

	return newone;
}
