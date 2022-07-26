#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 变量定义
 */
class AstNodeVarDef : public AstNode {
public:
	AstNodeVarDef(std::string var_name, TypeId declared_tid, AstNode* init_expr, bool is_const);

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);

private:
	std::string m_varname;
	/*
	 * 声明的类型
	 *		var name declared_type = 3;
	 * 如果缺省, 则为TYPE_ID_INFER. 在verify完成后设置为推得到的的实际类型
	 */
	TypeId	 m_declared_tid;
	AstNode* m_init_expr;
	bool	 m_is_const;
};
