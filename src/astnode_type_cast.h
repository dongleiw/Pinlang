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
 * 类型转换
 * Tsrc => Tdst
 */
class AstNodeTypeCast : public AstNode {
public:
	AstNodeTypeCast(AstNode* expr, AstNodeType* dst_type);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	AstNodeTypeCast* DeepCloneT();
	virtual AstNode* DeepClone() override { return DeepCloneT(); }

private:
	AstNode*	 m_expr;
	AstNodeType* m_dst_type;

	TypeId m_src_tid;
	TypeId m_dst_tid;
};
