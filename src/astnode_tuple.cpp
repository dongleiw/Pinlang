#include "astnode_tuple.h"
#include "astnode.h"
#include "define.h"
#include "log.h"
#include "type_mgr.h"
#include "verify_context.h"

#include <cassert>

VerifyContextResult AstNodeTuple::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	log_debug("verify tuple");

	std::vector<TypeId> element_tids;
	for (auto iter : m_expr_list) {
		element_tids.push_back(iter->Verify(ctx, VerifyContextParam()).GetResultTypeId());
	}

	m_result_typeid = g_typemgr.GetOrAddTypeTuple(ctx, element_tids);

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeTuple::Execute(ExecuteContext& ctx) {
	std::vector<Variable*> tuple_elements;
	for (auto iter : m_expr_list) {
		Variable* e = iter->Execute(ctx);
		tuple_elements.push_back(e);
	}
	Variable* v = Variable::CreateTypeTuple(m_result_typeid, tuple_elements);
	return v;
}
AstNodeTuple* AstNodeTuple::DeepCloneT() {
	AstNodeTuple* newone = new AstNodeTuple();
	newone->Copy(*this);

	for (auto iter : m_expr_list) {
		newone->m_expr_list.push_back(iter->DeepClone());
	}

	return newone;
}
