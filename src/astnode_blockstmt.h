#pragma once

#include <vector>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 一个block, 内部可以有任意数量的statment
 * 本身是statement而不是expression, 因此没有结果值
 * 如果该blockstatement在函数(方法)内, 则可以使用return返回值
 */
class AstNodeBlockStmt : public AstNode {
public:
	AstNodeBlockStmt(const std::vector<AstNode*>& stmts);

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);

	void AddPreDefine(AstNodeBlockStmt& another);
private:
	std::vector<AstNode*> m_predefine_stmts;
	std::vector<AstNode*> m_stmts;
};
