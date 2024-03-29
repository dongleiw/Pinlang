#include "astnode_if.h"
#include "astnode.h"
#include "astnode_blockstmt.h"
#include "compile_context.h"
#include "define.h"
#include "fntable.h"
#include "log.h"
#include "utils.h"
#include "verify_context.h"

#include <cassert>
#include <cstdio>
#include <llvm-12/llvm/IR/BasicBlock.h>
#include <llvm-12/llvm/IR/Instruction.h>

AstNodeIf::AstNodeIf(std::vector<AstNode*> cond_expr_list, std::vector<AstNode*> cond_block_list, AstNode* else_cond_block) {
	m_cond_expr_list = cond_expr_list;
	for (auto& iter : m_cond_expr_list) {
		iter->SetParent(this);
	}

	m_cond_block_list = cond_block_list;
	for (auto& iter : m_cond_block_list) {
		iter->SetParent(this);
	}

	m_else_cond_block = else_cond_block;
	if (m_else_cond_block != nullptr) {
		m_else_cond_block->SetParent(this);
	}
}
VerifyContextResult AstNodeIf::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	log_debug("verify if");
	assert(m_cond_expr_list.size() == m_cond_block_list.size());

	for (auto iter : m_cond_expr_list) {
		VerifyContextParam vparam_cond;
		vparam_cond.SetExpectResultTid(TYPE_ID_BOOL);
		VerifyContextResult vr = iter->Verify(ctx, vparam_cond);
		if (vr.GetResultTypeId() != TYPE_ID_BOOL) {
			panicf("result of conditional expr is not bool");
		}
	}
	for (auto iter : m_cond_block_list) {
		iter->Verify(ctx, VerifyContextParam().SetExpectReturnTid(vparam.GetExpectReturnTid()));
	}

	if (m_else_cond_block != nullptr) {
		m_else_cond_block->Verify(ctx, VerifyContextParam().SetExpectReturnTid(vparam.GetExpectResultTid()));
	}

	// 确定该if语句是否是exitnode. 规则为: if语句的所有分支都是exitnode
	if (m_cond_expr_list.size() + 1 == (m_cond_block_list.size() + (m_else_cond_block == nullptr ? 0 : 1))) {
		bool is_exit_node = true;
		for (auto b : m_cond_block_list) {
			if (!b->IsExitNode()) {
				is_exit_node = false;
			}
		}
		if (m_else_cond_block != nullptr) {
			if (!m_else_cond_block->IsExitNode()) {
				is_exit_node = false;
			}
		}
		m_is_exit_node = is_exit_node;
	} else {
		// 部分分支缺省. 则不可能是exitnode
		m_is_exit_node = false;
	}

	m_result_typeid = TYPE_ID_NONE;
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeIf::Execute(ExecuteContext& ctx) {
	panicf("");
	return nullptr;
}
AstNodeIf* AstNodeIf::DeepCloneT() {
	std::vector<AstNode*> cond_expr_list;
	std::vector<AstNode*> cond_block_list;
	AstNode*			  else_cond_block = nullptr;

	for (auto iter : m_cond_expr_list) {
		cond_expr_list.push_back(iter->DeepClone());
	}
	for (auto iter : m_cond_block_list) {
		cond_block_list.push_back(iter->DeepClone());
	}

	if (m_else_cond_block != nullptr)
		else_cond_block = m_else_cond_block->DeepClone();
	return new AstNodeIf(cond_expr_list, cond_block_list, else_cond_block);
}
/*
 * if a {
 * }
 *
 * if a {
 * }else{
 * }
 *
 * if a {
 * }else if b {
 * }
 *
 * if a {
 * }else if b {
 * }else{
 * }
 *
 * 如果所有分支都return了, 就不需要跳转到if后面的语句了
 *
 * if xx {
 *		if xxx {
 *			return 1;
 *		}else{
 *			return 2;
 *		}
 * }
 */
// CompileResult AstNodeIf::Compile(CompileContext& cctx) {
// 	std::vector<llvm::BasicBlock*> branchs;
// 	llvm::BasicBlock*			   block_after_if = nullptr;
// 	if (!m_is_exit_node) {
// 		block_after_if = llvm::BasicBlock::Create(IRC, "block_after_if", cctx.GetCurFn());
// 	}
//
// 	for (size_t i = 0; i < m_cond_expr_list.size(); i++) {
// 		CompileResult cr_cond_value = m_cond_expr_list.at(i)->Compile(cctx);
// 		assert(cr_cond_value.GetResult()->getType() == IRB.getInt1Ty());
//
// 		llvm::BasicBlock* true_block = llvm::BasicBlock::Create(IRC, sprintf_to_stdstr("if_branch_true_%lu", i), cctx.GetCurFn());
// 		branchs.push_back(true_block);
//
// 		llvm::BasicBlock* false_block = llvm::BasicBlock::Create(IRC, sprintf_to_stdstr("if_branch_false_%lu", i), cctx.GetCurFn());
// 		branchs.push_back(false_block);
//
// 		IRB.CreateCondBr(cr_cond_value.GetResult(), true_block, false_block);
// 	}
// 	for (size_t i = 0; i < m_cond_block_list.size(); i++) {
// 		IRB.SetInsertPoint(branchs.at(2 * i));
// 		m_cond_block_list.at(i)->Compile(cctx);
//
// 		if (m_cond_block_list.at(i)->IsExitNode()) {
// 			// 该分支为exitnode, 所以不需要再跳转了
// 		} else {
// 			IRB.CreateBr(block_after_if);
// 		}
// 	}
// 	IRB.SetInsertPoint(*branchs.rbegin());
// 	if (m_else_cond_block == nullptr) {
// 		IRB.CreateBr(block_after_if);
// 	} else {
// 		m_else_cond_block->Compile(cctx);
// 		if (m_else_cond_block->IsExitNode()) {
// 			// 该分支为exitnode, 所以不需要再跳转了
// 		} else {
// 			IRB.CreateBr(block_after_if);
// 		}
// 	}
//
// 	// 将入口设置为if后面的block, 让后续语句的指令放到后面
// 	// 如果该if语句为exitnode, 则无需再设置'后续block'
// 	if (!m_is_exit_node) {
// 		IRB.SetInsertPoint(block_after_if);
// 	}
// 	return CompileResult();
// }
CompileResult AstNodeIf::Compile(CompileContext& cctx) {
	std::vector<llvm::BasicBlock*> branchs;
	llvm::BasicBlock*			   block_after_if = nullptr;
	if (!m_is_exit_node) {
		block_after_if = llvm::BasicBlock::Create(IRC, "block_after_if", cctx.GetCurFn());
	}

	llvm::BasicBlock* previous_false_block = nullptr;

	for (size_t i = 0; i < m_cond_expr_list.size(); i++) {
		CompileResult cr_cond_value = m_cond_expr_list.at(i)->Compile(cctx);
		assert(cr_cond_value.GetResult()->getType() == IRB.getInt1Ty());

		llvm::BasicBlock* true_block = llvm::BasicBlock::Create(IRC, sprintf_to_stdstr("if_branch_true_%lu", i), cctx.GetCurFn());
		branchs.push_back(true_block);

		llvm::BasicBlock* false_block = llvm::BasicBlock::Create(IRC, sprintf_to_stdstr("if_branch_false_%lu", i), cctx.GetCurFn());
		branchs.push_back(false_block);

		IRB.CreateCondBr(cr_cond_value.GetResult(), true_block, false_block);

		IRB.SetInsertPoint(true_block);
		m_cond_block_list.at(i)->Compile(cctx);
		if(!m_cond_block_list.at(i)->IsExitNode()){
			IRB.CreateBr(block_after_if);
		}

		IRB.SetInsertPoint(false_block);
	}
	if (m_else_cond_block == nullptr) {
		IRB.CreateBr(block_after_if);
	} else {
		m_else_cond_block->Compile(cctx);
		if (m_else_cond_block->IsExitNode()) {
			// 该分支为exitnode, 所以不需要再跳转了
		} else {
			IRB.CreateBr(block_after_if);
		}
	}

	// 将入口设置为if后面的block, 让后续语句的指令放到后面
	// 如果该if语句为exitnode, 则无需再设置'后续block'
	if (!m_is_exit_node) {
		IRB.SetInsertPoint(block_after_if);
	}
	return CompileResult();
}
