#pragma once

#include <string>

#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "instruction.h"
#include "source_info.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * &address
 * get value of address
 */
class AstNodeReference : public AstNode {
public:
	AstNodeReference(AstNode* expr);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	AstNodeReference* DeepCloneT();
	virtual AstNode*  DeepClone() override { return DeepCloneT(); }

private:
	AstNodeReference() {}

private:
	AstNode* m_expr;
};
