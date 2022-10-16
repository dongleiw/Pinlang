#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 */
class AstNodeFor : public AstNode {
public:
	AstNodeFor(AstNode* init_expr, AstNode* cond_expr, AstNode* loop_expr, AstNode* body) {
		m_init_expr = init_expr;
		m_cond_expr = cond_expr;
		m_loop_expr = loop_expr;
		m_body		= body;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeFor*		 DeepCloneT();

private:
	AstNodeFor() {}

private:
	AstNode* m_init_expr;
	AstNode* m_cond_expr;
	AstNode* m_loop_expr;
	AstNode* m_body;
};
