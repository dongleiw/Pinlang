#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 变量定义
 */
class AstNodeFnCall : public AstNode {
public:
	AstNodeFnCall(AstNode* fn_expr, std::vector<AstNode*> args);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeFnCall*	 DeepCloneT();

private:
	AstNodeFnCall() {}

private:
	AstNode*			  m_fn_expr;
	std::vector<AstNode*> m_args;
};
