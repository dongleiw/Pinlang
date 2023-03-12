#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "compile_context.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * continue语句
 */
class AstNodeContinue : public AstNode {
public:
	AstNodeContinue();

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeContinue* DeepCloneT();

private:
};
