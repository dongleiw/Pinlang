#pragma once

#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

class AstNodeLiteralFloat: public AstNode {
public:
	AstNodeLiteralFloat(AstNodeType* type, double value);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeLiteralFloat*	 DeepCloneT();

	virtual bool IsLiteral() const override { return true; }
private:
	AstNodeType* m_type; // 显式类型. 缺省为null
	double m_value;
};
