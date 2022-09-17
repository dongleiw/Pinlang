#include "astnode_if.h"
#include "define.h"
#include "log.h"
#include "verify_context.h"

#include <cassert>

VerifyContextResult AstNodeIf::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify if");
	assert(m_cond_expr_list.size() == m_cond_block_list.size());

	for (auto iter : m_cond_expr_list) {
		VerifyContextParam vparam_cond;
		vparam_cond.SetResultTid(TYPE_ID_BOOL);
		VerifyContextResult vr = iter->Verify(ctx, vparam_cond);
		if (vr.GetResultTypeId() != TYPE_ID_BOOL) {
			panicf("result of conditional expr is not bool");
		}
	}
	for (auto iter : m_cond_block_list) {
		iter->Verify(ctx,VerifyContextParam());
	}

	if (m_else_cond_block != nullptr) {
		m_else_cond_block->Verify(ctx,VerifyContextParam());
	}

	m_result_typeid = TYPE_ID_NONE;
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeIf::Execute(ExecuteContext& ctx) {
	for (size_t i = 0; i < m_cond_expr_list.size(); i++) {
		Variable* v = m_cond_expr_list.at(i)->Execute(ctx);
		if (v->GetValueBool()) {
			m_cond_block_list.at(i)->Execute(ctx);
			return nullptr;
		}
	}

	if (m_else_cond_block != nullptr) {
		m_else_cond_block->Execute(ctx);
	}
	return nullptr;
}
AstNodeIf* AstNodeIf::DeepCloneT() {
	std::vector<AstNode*> cond_expr_list;
	std::vector<AstNode*> cond_block_list;
	AstNode*			  else_cond_block;

	for (auto iter : m_cond_expr_list) {
		cond_expr_list.push_back(iter->DeepClone());
	}
	for (auto iter : m_cond_block_list) {
		cond_block_list.push_back(iter->DeepClone());
	}

	if (m_else_cond_block)
		else_cond_block = m_else_cond_block->DeepClone();
	return new AstNodeIf(cond_expr_list, cond_block_list, else_cond_block);
}
