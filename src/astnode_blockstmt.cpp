#include "astnode_blockstmt.h"
#include "astnode_break.h"
#include "astnode_class_def.h"
#include "astnode_complex_fndef.h"
#include "astnode_return.h"
#include "astnode_vardef.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "variable_table.h"
#include "verify_context.h"
#include <cassert>
#include <utility>

AstNodeBlockStmt::AstNodeBlockStmt() {
	m_global_block = false;
}
AstNodeBlockStmt::AstNodeBlockStmt(const std::vector<AstNode*>& stmts) {
	m_stmts = stmts;
	for (auto iter : m_stmts) {
		iter->SetParent(this);
	}
	m_global_block = false;
}
VerifyContextResult AstNodeBlockStmt::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	verify_begin();

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

	if (m_global_block) {
		ctx.GetCurStack()->EnterBlock(ctx.GetGlobalVt());
	} else {
		ctx.GetCurStack()->EnterBlock(new VariableTable());
	}
	for (auto n : m_stmts) {
		if (n->IsVerified()) {
			continue;
		}
		n->Verify(ctx, VerifyContextParam().SetReturnTid(vparam.GetReturnTid()));
	}
	for (auto dep : m_dependency) {
		for (auto iter = m_stmts.begin(); iter != m_stmts.end(); iter++) {
			if (dep.second == *iter) {
				m_stmts.erase(iter);
				break;
			}
		}
		for (auto iter = m_stmts.begin(); iter != m_stmts.end(); iter++) {
			if (dep.first == *iter) {
				m_stmts.insert(iter, dep.second);
				break;
			}
		}
	}
	ctx.GetCurStack()->LeaveBlock();

	// 确定该blockstatement是否是exitnode
	if (!m_stmts.empty() && (*m_stmts.rbegin())->IsExitNode()) {
		m_is_exit_node = true;
	}

	verify_end();
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

	if (m_global_block) {
		ctx.GetCurStack()->EnterBlock(ctx.GetGlobalVt());
	} else {
		ctx.GetCurStack()->EnterBlock(new VariableTable());
	}

	for (auto n : m_stmts) {
		n->Execute(ctx);
		if (ctx.GetCurStack()->HasReturned() || ctx.GetCurStack()->IsBreaked()) {
			break;
		}
	}
	ctx.GetCurStack()->LeaveBlock();
	return nullptr;
}
void AstNodeBlockStmt::AddPreDefine(AstNodeBlockStmt& another) {
	m_predefine_stmts = another.m_stmts;
	another.m_stmts.clear();
}
AstNodeBlockStmt* AstNodeBlockStmt::DeepCloneT() {
	assert(m_predefine_stmts.empty() && m_global_block == false);

	std::vector<AstNode*> stmts;
	for (auto iter : m_stmts) {
		stmts.push_back(iter->DeepClone());
	}

	AstNodeBlockStmt* newone = new AstNodeBlockStmt(stmts);
	return newone;
}
void AstNodeBlockStmt::AddChildStmt(AstNode* node) {
	m_stmts.push_back(node);
}
void AstNodeBlockStmt::MergeAnother(AstNodeBlockStmt& another) {
	assert(this != &another);
	for (auto iter : another.m_stmts) {
		iter->SetParent(this);
		m_stmts.push_back(iter);
	}
	another.m_stmts.clear();
}
void AstNodeBlockStmt::VerifyIdentfier(AstNode* cur_node, std::string id, VerifyContext& ctx, VerifyContextParam vparam) {
	assert(m_global_block == true);
	// 找到当前变量
	while (cur_node->GetParent() != nullptr && cur_node->GetParent() != this) {
		cur_node = cur_node->GetParent();
	}
	if (cur_node == nullptr) {
		panicf("bug");
	}

	// 依赖
	for (size_t i = 0; i < m_stmts.size(); i++) {
		AstNode*	   astnode		  = m_stmts.at(i);
		AstNodeVarDef* astnode_vardef = dynamic_cast<AstNodeVarDef*>(astnode);
		if (astnode_vardef != nullptr && astnode_vardef->GetVarName() == id) {
			m_dependency.push_back(std::pair(cur_node, astnode));
			astnode_vardef->Verify(ctx, vparam);
			return;
		}
		AstNodeClassDef* astnode_classdef = dynamic_cast<AstNodeClassDef*>(astnode);
		if (astnode_classdef != nullptr && astnode_classdef->GetClassName() == id) {
			m_dependency.push_back(std::pair(cur_node, astnode));
			astnode_classdef->Verify(ctx, vparam);
			return;
		}
		AstNodeComplexFnDef* astnode_fndef = dynamic_cast<AstNodeComplexFnDef*>(astnode);
		if (astnode_fndef != nullptr && astnode_fndef->GetName() == id) {
			m_dependency.push_back(std::pair(cur_node, astnode));
			astnode_fndef->Verify(ctx, vparam);
			return;
		}
	}
	// panicf("global identifier[%s] not exists", id.c_str());
}
llvm::Value* AstNodeBlockStmt::Compile(LLVMIR& llvm_ir) {
	for (auto node : m_stmts) {
		node->Compile(llvm_ir);
	}
	return nullptr;
}
