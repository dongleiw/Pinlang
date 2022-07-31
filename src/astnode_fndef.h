#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 变量定义
 */
class AstNodeFnDef : public AstNode {
public:
	AstNodeFnDef(TypeId tid, std::string fn_name, std::vector<std::string> params_name, AstNodeBlockStmt* body);

	virtual VerifyContextResult Verify(VerifyContext& ctx)override;
	virtual Variable*			Execute(ExecuteContext& ctx)override;

private:
	TypeId					 m_tid;
	std::string				 m_fnname;
	std::vector<std::string> m_params_name;
	AstNodeBlockStmt*				 m_body;
};
