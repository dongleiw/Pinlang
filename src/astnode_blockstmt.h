#pragma once

#include <vector>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

class AstNodeBlockStmt : public AstNode {
public:
	AstNodeBlockStmt(const std::vector<AstNode*>& stmts);

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);
	virtual Variable*			GetConstantValue(){return nullptr; }

private:
	std::vector<AstNode*> m_stmts;
};
