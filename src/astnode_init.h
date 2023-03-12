#pragma once

#include <string>

#include "astnode.h"
#include "astnode_type.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 初始化表达式
 *		- 数组元素初始化
 *		- class的属性初始化
 */
class AstNodeInit : public AstNode {
public:
	AstNodeInit(std::vector<ParserInitElement> elements) {
		m_type	   = nullptr;
		m_elements = elements;
	}

	void SetType(AstNodeType* type) {
		m_type = type;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vr_param) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeInit*	 DeepCloneT();

private:
	AstNodeInit() {}

private:
	AstNodeType*				   m_type; // 如果类型缺省, 则为null
	std::vector<ParserInitElement> m_elements;
};
