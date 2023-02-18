#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 */
class AstNodeIf : public AstNode {
public:
	AstNodeIf(std::vector<AstNode*> cond_expr_list, std::vector<AstNode*> cond_block_list, AstNode* else_cond_block);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual llvm::Value*		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeIf*		 DeepCloneT();

private:
	AstNodeIf() {}

private:
	std::vector<AstNode*> m_cond_expr_list;
	std::vector<AstNode*> m_cond_block_list;
	AstNode*			  m_else_cond_block; // optional
};
