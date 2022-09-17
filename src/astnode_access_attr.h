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
class AstNodeAccessAttr : public AstNode {
public:
	AstNodeAccessAttr(AstNode* obj_expr, std::string attr_name) : m_obj_expr(obj_expr), m_attr_name(attr_name) {
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);

private:
	AstNode*	m_obj_expr;
	std::string m_attr_name;

	int m_attr_idx;
};
