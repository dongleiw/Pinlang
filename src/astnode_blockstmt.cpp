#include "astnode_blockstmt.h"
#include "astnode_break.h"
#include "astnode_class_def.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_return.h"
#include "astnode_vardef.h"
#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"
#include <cassert>

AstNodeBlockStmt::AstNodeBlockStmt() {
	m_is_global_block = false;
}
AstNodeBlockStmt::AstNodeBlockStmt(const std::vector<AstNode*>& stmts) {
	m_stmts = stmts;
	for (auto iter : m_stmts) {
		iter->SetParent(this);
	}
	m_is_global_block = false;
}
VerifyContextResult AstNodeBlockStmt::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	VerifyContextResult vr(m_result_typeid);

	if (!m_is_global_block) {
		ctx.GetCurStack()->EnterBlock(new VariableTable());
	}
	for (auto n : m_stmts) {
		if (n->IsVerified()) {
			continue;
		}
		n->Verify(ctx, VerifyContextParam().SetExpectReturnTid(vparam.GetExpectReturnTid()));
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

	if (!m_is_global_block) {
		ctx.GetCurStack()->LeaveBlock();
	}

	// 确定该blockstatement是否是exitnode
	if (!m_stmts.empty() && (*m_stmts.rbegin())->IsExitNode()) {
		m_is_exit_node = true;
	}

	return vr;
}
Variable* AstNodeBlockStmt::Execute(ExecuteContext& ctx) {

	if (m_is_global_block) {
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
AstNodeBlockStmt* AstNodeBlockStmt::DeepCloneT() {
	assert(m_is_global_block == false);

	std::vector<AstNode*> stmts;
	for (auto iter : m_stmts) {
		stmts.push_back(iter->DeepClone());
	}

	AstNodeBlockStmt* newone = new AstNodeBlockStmt(stmts);
	return newone;
}
void AstNodeBlockStmt::AddChildStmt(AstNode* node) {
	m_stmts.push_back(node);
	node->SetParent(this);
}
void AstNodeBlockStmt::MergeAnother(AstNodeBlockStmt& another) {
	assert(this != &another);
	for (auto iter : another.m_stmts) {
		iter->SetParent(this);
		m_stmts.push_back(iter);
	}
	another.m_stmts.clear();
}
bool AstNodeBlockStmt::VerifyGlobalIdentifier(std::string id, VerifyContext& ctx, VerifyContextParam vparam) {
	assert(m_is_global_block == true);
	// 找到当前变量
	//while (cur_node->GetParent() != nullptr && cur_node->GetParent() != this) {
	//	cur_node = cur_node->GetParent();
	//}
	//if (cur_node == nullptr) {
	//	panicf("bug");
	//}

	// 依赖
	for (size_t i = 0; i < m_stmts.size(); i++) {
		AstNode*	   astnode		  = m_stmts.at(i);
		AstNodeVarDef* astnode_vardef = dynamic_cast<AstNodeVarDef*>(astnode);
		if (astnode_vardef != nullptr && astnode_vardef->GetVarName() == id) {
			//m_dependency.push_back(std::pair(cur_node, astnode));
			astnode_vardef->Verify(ctx, vparam);
			return true;
		}
		AstNodeClassDef* astnode_classdef = dynamic_cast<AstNodeClassDef*>(astnode);
		if (astnode_classdef != nullptr && astnode_classdef->GetClassName() == id) {
			//m_dependency.push_back(std::pair(cur_node, astnode));
			astnode_classdef->Verify(ctx, vparam);
			return true;
		}
		AstNodeComplexFnDef* astnode_fndef = dynamic_cast<AstNodeComplexFnDef*>(astnode);
		if (astnode_fndef != nullptr && astnode_fndef->GetName() == id) {
			//m_dependency.push_back(std::pair(cur_node, astnode));
			astnode_fndef->Verify(ctx, vparam);
			return true;
		}
		AstNodeConstraint* astnode_constraint = dynamic_cast<AstNodeConstraint*>(astnode);
		if (astnode_constraint != nullptr && astnode_constraint->GetName() == id) {
			//m_dependency.push_back(std::pair(cur_node, astnode));
			astnode_constraint->Verify(ctx, vparam);
			return true;
		}
	}
	return false;
	// panicf("global identifier[%s] not exists", id.c_str());
}
bool AstNodeBlockStmt::HasGlobalDef(std::string id) const {
	assert(m_is_global_block == true);
	for (size_t i = 0; i < m_stmts.size(); i++) {
		AstNode*	   astnode		  = m_stmts.at(i);
		AstNodeVarDef* astnode_vardef = dynamic_cast<AstNodeVarDef*>(astnode);
		if (astnode_vardef != nullptr && astnode_vardef->GetVarName() == id) {
			return true;
		}
		AstNodeClassDef* astnode_classdef = dynamic_cast<AstNodeClassDef*>(astnode);
		if (astnode_classdef != nullptr && astnode_classdef->GetClassName() == id) {
			return true;
		}
		AstNodeComplexFnDef* astnode_fndef = dynamic_cast<AstNodeComplexFnDef*>(astnode);
		if (astnode_fndef != nullptr && astnode_fndef->GetName() == id) {
			return true;
		}
	}
	return false;
}
CompileResult AstNodeBlockStmt::Compile(CompileContext& cctx) {
	cctx.EnterBlock();
	for (auto node : m_stmts) {
		node->Compile(cctx);
	}
	cctx.LeaveBlock();
	return CompileResult();
}
