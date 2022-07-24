#pragma once

#include <vector>

#include "astnode.h"
#include "type.h"
#include "verify_context.h"
#include "execute_context.h"
#include "variable.h"

class AstNodeBlockStmt: public AstNode{
public:
	AstNodeBlockStmt(const std::vector<AstNode*> &stmts);

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable* Execute(ExecuteContext& ctx);

private:
	std::vector<AstNode*> m_stmts;
};
