#include "astnode_blockstmt.h"
#include "astnode_return.h"
#include "type.h"
#include "type_mgr.h"
#include "variable_table.h"
#include "verify_context.h"

AstNodeBlockStmt::AstNodeBlockStmt(const std::vector<AstNode*>& stmts) {
	m_stmts = stmts;
	for (auto iter : m_stmts) {
		iter->SetParent(this);
	}
}
VerifyContextResult AstNodeBlockStmt::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VerifyContextResult vr(m_result_typeid);

	if (!m_predefine_stmts.empty()) {
		// 检查预定义内容
		// 这些内容需要在globalVt中, 因此需要在EnterBlock外面
		for (auto n : m_predefine_stmts) {
			n->Verify(ctx, VerifyContextParam());
		}
		// 给基础类型添加constraint实现
		g_typemgr.InitBuiltinMethods(ctx);
	}

	ctx.GetCurStack()->EnterBlock(new VariableTable());
	for (auto n : m_stmts) {
		AstNodeReturn* astnode_return = dynamic_cast<AstNodeReturn*>(n);
		if (astnode_return != nullptr) {
			n->Verify(ctx, VerifyContextParam().SetReturnTid(vparam.GetReturnTid()));
		} else {
			n->Verify(ctx, VerifyContextParam());
		}
	}
	ctx.GetCurStack()->LeaveBlock();
	return vr;
}
Variable* AstNodeBlockStmt::Execute(ExecuteContext& ctx) {
	if (!m_predefine_stmts.empty()) {
		// 执行预定义内容
		// 这些内容需要在globalVt中, 因此需要在EnterBlock外面
		for (auto n : m_predefine_stmts) {
			n->Execute(ctx);
		}
	}

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
AstNodeBlockStmt* AstNodeBlockStmt::DeepCloneT() {
	AstNodeBlockStmt* newone = new AstNodeBlockStmt(std::vector<AstNode*>());

	for (auto iter : m_predefine_stmts) {
		newone->m_predefine_stmts.push_back(iter->DeepClone());
	}
	for (auto iter : m_stmts) {
		newone->m_stmts.push_back(iter->DeepClone());
	}

	return newone;
}
void AstNodeBlockStmt::AddChildStmt(AstNode* node){
	m_stmts.push_back(node);
}
