#include "verify_context.h"
#include "astnode_blockstmt.h"
#include "type.h"
#include "type_mgr.h"

#include <assert.h>

VerifyContext::VerifyContext(std::vector<AstNodeBlockStmt*> global_block_stmts) {
	m_global_vt.InitAsGlobal();
	m_global_block_stmts = global_block_stmts;
}

void VerifyContext::PushStack() {
	Stack* new_stack = new Stack(m_top_stack);
	new_stack->EnterBlock(&m_global_vt);
	m_top_stack = new_stack;
}
void VerifyContext::PopStack() {
	assert(m_top_stack != nullptr);
	m_top_stack = m_top_stack->GetNextStack();
}
void VerifyContext::Verify() {
	PushStack();
	bool predefine_verified = false;
	for (auto global_block_stmt : m_global_block_stmts) {
		log_info("begin to verify. srcfile[%s]", global_block_stmt->GetSrcFilename().c_str());
		global_block_stmt->Verify(*this, VerifyContextParam());
		if (!predefine_verified) {
			// 给基础类型添加constraint实现
			g_typemgr.InitBuiltinMethods(*this);
			predefine_verified = true;
		}
		log_info("verify end. srcfile[%s]", global_block_stmt->GetSrcFilename().c_str());
	}
}
void VerifyContext::VerifyGlobalIdentifier(const AstNode* cur_node, std::string id, VerifyContextParam vparam) {
	PushStack();
	for (auto global_block_stmt : m_global_block_stmts) {
		if (global_block_stmt->VerifyGlobalIdentifier(id, *this, VerifyContextParam())) {
			break;
		}
	}
	PopStack();
}
