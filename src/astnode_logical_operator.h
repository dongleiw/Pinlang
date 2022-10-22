#pragma once

#include <string>

#include "astnode.h"
#include "define.h"
#include "execute_context.h"
#include "source_info.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 操作符: and(&&) or(||) not(!)
 */
class AstNodeLogicalOperator : public AstNode {
public:
	AstNodeLogicalOperator(AstNode* left_expr, std::string op, AstNode* right_expr);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	AstNodeLogicalOperator* DeepCloneT();
	virtual AstNode* DeepClone() override { return DeepCloneT(); }

private:
	AstNode*	m_left_expr;
	std::string m_op;
	AstNode*	m_right_expr;
};
