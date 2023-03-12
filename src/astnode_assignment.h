#pragma once

#include <string>

#include "astnode.h"
#include "compile_context.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 赋值
 */
class AstNodeAssignment : public AstNode {
public:
	AstNodeAssignment(AstNode* left, AstNode* right) {
		m_left	= left;
		m_right = right;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vr_param) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode*   DeepClone() override { return DeepCloneT(); }
	AstNodeAssignment* DeepCloneT();

private:
	AstNodeAssignment() {}

private:
	AstNode* m_left;
	AstNode* m_right;
};
