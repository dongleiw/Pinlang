#pragma once

#include "astnode.h"
#include "type.h"
#include "verify_context.h"
#include "execute_context.h"
#include "variable.h"

class AstNodeLiteral : public AstNode{
public:
	AstNodeLiteral(int value);

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable* Execute(ExecuteContext& ctx);

private:
	int m_value_int;
};
