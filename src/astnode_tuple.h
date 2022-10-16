#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 元组 tuple
 *		(1, 2.3, "hello")
 */
class AstNodeTuple : public AstNode {
public:
	AstNodeTuple(std::vector<AstNode*> expr_list) {
		m_expr_list = expr_list;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeTuple*	 DeepCloneT();

private:
	AstNodeTuple() {}

private:
	std::vector<AstNode*> m_expr_list;
};
