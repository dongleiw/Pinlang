#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 操作符: + - * / %
 * 将操作符转换为(某个restriction的)方法的调用
 */
class AstNodeOperator : public AstNode {
public:
	AstNodeOperator(AstNode* left_expr, std::string restriction_name, std::string op, AstNode* right_expr);

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);

private:
	AstNode*	m_left_expr;
	std::string m_restriction_name;
	std::string m_op;
	int			m_method_idx;
	AstNode*	m_right_expr;
};
