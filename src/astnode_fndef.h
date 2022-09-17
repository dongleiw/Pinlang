#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_type.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 变量定义
 */
class AstNodeFnDef : public AstNode {
public:
	AstNodeFnDef(std::string fn_name, std::vector<ParserParameter> params, AstNodeType* return_type, AstNodeBlockStmt* body);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeFnDef*	 DeepCloneT();

private:
	AstNodeFnDef() {}

private:
	std::string					 m_fnname;
	std::vector<ParserParameter> m_params;
	AstNodeType*				 m_return_type;
	AstNodeBlockStmt*			 m_body;

	std::string m_uniq_fnname;
};
