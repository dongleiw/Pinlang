#include "astnode_for.h"
#include "compile_context.h"
#include "define.h"
#include "log.h"
#include "variable_table.h"
#include "verify_context.h"

#include <cassert>
#include <llvm-12/llvm/IR/BasicBlock.h>

AstNodeFor::AstNodeFor(AstNode* init_expr, AstNode* cond_expr, AstNode* loop_expr, AstNode* body) {
	m_init_expr = init_expr;
	m_cond_expr = cond_expr;
	m_loop_expr = loop_expr;
	m_body		= body;

	if (m_init_expr != nullptr) {
		m_init_expr->SetParent(this);
	}
	if (m_cond_expr != nullptr) {
		m_cond_expr->SetParent(this);
	}
	if (m_loop_expr != nullptr) {
		m_loop_expr->SetParent(this);
	}
	m_body->SetParent(this);
}
VerifyContextResult AstNodeFor::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify for");
	ctx.GetCurStack()->EnterBlock(new VariableTable());

	if (m_init_expr != nullptr) {
		m_init_expr->Verify(ctx, VerifyContextParam());
	}
	if (m_cond_expr != nullptr) {
		VerifyContextResult vr_cond = m_cond_expr->Verify(ctx, VerifyContextParam().SetResultTid(TYPE_ID_BOOL));
		if (vr_cond.GetResultTypeId() != TYPE_ID_BOOL) {
			panicf("type of cond expr is not bool");
		}
	}
	if (m_loop_expr != nullptr) {
		m_loop_expr->Verify(ctx, VerifyContextParam());
	}

	m_body->Verify(ctx, VerifyContextParam().SetReturnTid(vparam.GetReturnTid()));

	ctx.GetCurStack()->LeaveBlock();

	m_result_typeid = TYPE_ID_NONE;
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeFor::Execute(ExecuteContext& ctx) {
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	if (m_init_expr != nullptr) {
		m_init_expr->Execute(ctx);
	}
	while (true) {
		if (m_cond_expr != nullptr) {
			Variable* v_cond = m_cond_expr->Execute(ctx);
			if (v_cond->GetValueBool() == false) {
				break;
			}
		}

		m_body->Execute(ctx);
		if (ctx.GetCurStack()->HasReturned() || ctx.GetCurStack()->IsBreaked()) {
			break;
		}

		if (m_loop_expr != nullptr) {
			m_loop_expr->Execute(ctx);
		}
	}
	ctx.GetCurStack()->SetBreaked(false); // 清掉break
	ctx.GetCurStack()->LeaveBlock();
	return nullptr;
}
AstNodeFor* AstNodeFor::DeepCloneT() {
	return new AstNodeFor(m_init_expr == nullptr ? nullptr : m_init_expr->DeepClone(),
						  m_cond_expr == nullptr ? nullptr : m_cond_expr->DeepClone(),
						  m_loop_expr == nullptr ? nullptr : m_loop_expr->DeepClone(),
						  m_body->DeepClone());
}
CompileResult AstNodeFor::Compile(CompileContext& cctx) {
	if (m_init_expr != nullptr) {
		m_init_expr->Compile(cctx);
	}
	llvm::BasicBlock* for_cond_block = llvm::BasicBlock::Create(IRC, "for_cond", cctx.GetCurFn());
	llvm::BasicBlock* for_loop_block = llvm::BasicBlock::Create(IRC, "for_loop", cctx.GetCurFn());
	llvm::BasicBlock* for_body_block = llvm::BasicBlock::Create(IRC, "for_body", cctx.GetCurFn());
	llvm::BasicBlock* for_end_block	 = llvm::BasicBlock::Create(IRC, "for_end", cctx.GetCurFn());

	IRB.CreateBr(for_cond_block);
	IRB.SetInsertPoint(for_cond_block);

	if (m_cond_expr != nullptr) {
		CompileResult cr_cond_value = m_cond_expr->Compile(cctx);
		IRB.CreateCondBr(cr_cond_value.GetResult(), for_body_block, for_end_block);
	} else {
		IRB.CreateBr(for_body_block);
	}

	IRB.SetInsertPoint(for_body_block);
	// for statement允许continue/break. 需要设置对应跳转block
	llvm::BasicBlock* old_continue_block = cctx.GetContinueBlock();
	llvm::BasicBlock* old_break_block	 = cctx.GetBreakBlock();
	cctx.SetContinueBlock(for_loop_block);
	cctx.SetBreakBlock(for_end_block);
	m_body->Compile(cctx);
	cctx.SetContinueBlock(old_continue_block);
	cctx.SetBreakBlock(old_break_block);
	IRB.CreateBr(for_loop_block);

	IRB.SetInsertPoint(for_loop_block);
	if (m_loop_expr != nullptr) {
		m_loop_expr->Compile(cctx);
	}
	IRB.CreateBr(for_cond_block);

	IRB.SetInsertPoint(for_end_block);
	return CompileResult();
}
