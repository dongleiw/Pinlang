#pragma once

#include <string>

#include "astnode.h"
#include "astnode_type.h"
#include "compile_context.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 变量定义
 */
class AstNodeVarDef : public AstNode {
public:
	AstNodeVarDef(std::string var_name, AstNodeType* declared_type, AstNode* init_expr, bool is_const);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeVarDef*	 DeepCloneT();

	std::string GetVarName() const { return m_varname; }

private:
	AstNodeVarDef() {
		m_declared_type = nullptr;
		m_init_expr		= nullptr;
		m_is_const		= false;
	}

private:
	std::string m_varname;
	/*
	 * 声明的类型
	 *		var name declared_type = 3;
	 * 如果缺省, 则为nullptr
	 */
	AstNodeType* m_declared_type;
	AstNode*	 m_init_expr;
	bool		 m_is_const;
};
