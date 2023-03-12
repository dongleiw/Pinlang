#pragma once

#include "astnode.h"
#include "compile_context.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

class AstNodeLiteral : public AstNode {
public:
	AstNodeLiteral() {}
	AstNodeLiteral(int32_t value);
	AstNodeLiteral(int64_t value);
	AstNodeLiteral(float value);
	AstNodeLiteral(bool value);
	AstNodeLiteral(std::string value);

	void SetNullPointer();

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeLiteral*	 DeepCloneT();

	void CastToInt64();

private:
private:
	uint64_t	m_value_int;
	float		m_value_float;
	bool		m_value_bool;
	std::string m_value_str;
};
