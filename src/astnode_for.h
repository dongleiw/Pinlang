#pragma once

#include <string>

#include "astnode.h"
#include "compile_context.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 */
class AstNodeFor : public AstNode {
public:
	AstNodeFor(AstNode* init_expr, AstNode* cond_expr, AstNode* loop_expr, AstNode* body);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeFor*		 DeepCloneT();

private:
	AstNode* m_init_expr; // 初始化部分, 可能为null
	AstNode* m_cond_expr; // 条件部分, 可能为null
	AstNode* m_loop_expr; // 循环部分, 可能为null
	AstNode* m_body;
};
