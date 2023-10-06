#pragma once

#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

class AstNodeLiteralBool : public AstNode {
public:
	AstNodeLiteralBool(bool value);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode*	DeepClone() override { return DeepCloneT(); }
	AstNodeLiteralBool* DeepCloneT();

	virtual bool IsLiteral() const override { return true; }

private:
	bool m_value;
};
