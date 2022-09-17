#include "astnode_blockstmt.h"
#include "type.h"
#include "type_mgr.h"
#include "variable_table.h"
#include "verify_context.h"

AstNodeBlockStmt::AstNodeBlockStmt(const std::vector<AstNode*>& stmts) {
	m_result_typeid = TYPE_ID_NONE;
	for (size_t i = 0; i < stmts.size(); i++) {
		m_stmts.push_back(stmts.at(i));
	}
}
VerifyContextResult AstNodeBlockStmt::Verify(VerifyContext& ctx) {
	VerifyContextResult vr(m_result_typeid);

	ctx.GetCurStack()->EnterBlock(new VariableTable());
	if (!m_predefine_stmts.empty()) {
		// 加载预定义内容
		for (auto n : m_predefine_stmts) {
			n->Verify(ctx);
		}
		// 给基础类型添加restriction实现
		g_typemgr.InitBuiltinMethods(ctx);
	}

	for (auto n : m_stmts) {
		n->Verify(ctx);
	}
	ctx.GetCurStack()->LeaveBlock();
	return vr;
}
Variable* AstNodeBlockStmt::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->EnterBlock(new VariableTable());

	for (auto n : m_stmts) {
		n->Execute(ctx);
	}
	ctx.GetCurStack()->LeaveBlock();
	return nullptr;
}
void AstNodeBlockStmt::AddPreDefine(AstNodeBlockStmt& another) {
	m_predefine_stmts = another.m_stmts;
	another.m_stmts.clear();
}
