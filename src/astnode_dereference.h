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
 * &value
 * get address of value
 */
class AstNodeDereference : public AstNode {
public:
	AstNodeDereference(AstNode* value_expr);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	AstNodeDereference* DeepCloneT();
	virtual AstNode*	DeepClone() override { return DeepCloneT(); }

private:
	AstNodeDereference() {}

private:
	AstNode* m_value_expr;
};
