#include "astnode_blockstmt.h"
#include "type.h"
#include "verify_context.h"

AstNodeBlockStmt::AstNodeBlockStmt(const std::vector<AstNode*>& stmts) {
	m_result_typeid = TYPE_ID_NONE;
	for (size_t i = 0; i < stmts.size(); i++) {
		m_stmts.push_back(stmts.at(i));
	}
}
VerifyContextResult AstNodeBlockStmt::Verify(VerifyContext& ctx) {
	VerifyContextResult vr(m_result_typeid);

	for (auto n : m_stmts) {
		n->Verify(ctx);
	}
	return vr;
}
Variable* AstNodeBlockStmt::Execute(ExecuteContext& ctx) {
	for (auto n : m_stmts) {
		n->Execute(ctx);
	}
	return NULL;
}
