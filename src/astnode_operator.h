#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 操作符: + - * / %
 * 操作符可以转换为方法调用, 但是目前还没不支持方法
 */
class AstNodeOperator : public AstNode {
public:
	AstNodeOperator(AstNode* left_expr, std::string op, AstNode* right_expr);

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);

private:
	AstNode*	m_left_expr;
	std::string m_op;
	std::string m_uniq_method_name;
	AstNode*	m_right_expr;
};
