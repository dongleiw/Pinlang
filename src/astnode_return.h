#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * return语句
 */
class AstNodeReturn : public AstNode {
public:
	AstNodeReturn(AstNode* returned_expr);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeReturn*	 DeepCloneT();

private:
	AstNodeReturn() {}

private:
	AstNode* m_returned_expr;
	TypeId	 m_returned_expr_tid;
	bool	 m_returned_expr_is_tmp;
};
