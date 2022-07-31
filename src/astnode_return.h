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

	virtual VerifyContextResult Verify(VerifyContext& ctx)override;
	virtual Variable*			Execute(ExecuteContext& ctx)override;

private:
	AstNode* m_returned_expr;
};
