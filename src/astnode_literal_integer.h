#pragma once

#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

class AstNodeLiteralInteger : public AstNode {
public:
	AstNodeLiteralInteger(AstNodeType* type, int64_t value);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeLiteralInteger*	 DeepCloneT();

	void CastToInt64();

	virtual bool IsLiteral() const override { return true; }

private:
	AstNodeType* m_type; // 显式类型. 缺省为null
	uint64_t	 m_value;
};
