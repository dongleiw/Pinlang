#pragma once

#include <string>

#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 通过下标访问数组元素
 */
class AstNodeAccessArrayElement : public AstNode {
public:
	AstNodeAccessArrayElement(AstNode* array_expr, AstNode* index_expr) {
		m_array_expr = array_expr;
		m_index_expr = index_expr;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vr_param) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode*		   DeepClone() override { return DeepCloneT(); }
	AstNodeAccessArrayElement* DeepCloneT();

private:
	AstNodeAccessArrayElement() {}

private:
	AstNode* m_array_expr;
	AstNode* m_index_expr;

	TypeId m_array_tid;
};
