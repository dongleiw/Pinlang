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
class AstNodeReturn : public AstNode {
public:
	AstNodeReturn(AstNode* returned_expr);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeReturn*	 DeepCloneT();

private:
	AstNodeReturn() {}

private:
	AstNode* m_returned_expr;
};
