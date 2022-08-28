#pragma once

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

class AstNodeLiteral : public AstNode {
public:
	AstNodeLiteral(int value);
	AstNodeLiteral(std::string value);

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);
	//virtual Variable*			GetConstantValue();

private:
	int m_value_int;
	std::string m_value_str;
};
