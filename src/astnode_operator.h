#pragma once

#include <string>

#include "astnode.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "instruction.h"
#include "source_info.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 操作符: + - * / % >= <= > < != []
 * 将操作符转换为(某个constraint的)方法的调用
 */
class AstNodeOperator : public AstNode {
public:
	AstNodeOperator(AstNode* left_expr, std::string constraint_name, std::string op, AstNode* right_expr);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual llvm::Value*		Compile(LLVMIR& llvm_ir) override;

	AstNodeOperator* DeepCloneT();
	virtual AstNode* DeepClone() override { return DeepCloneT(); }

	void SetSourceInfo_op(SourceInfo si) { m_si_op = si; }

private:
	AstNode*	m_left_expr;
	std::string m_constraint_name;
	std::string m_op;
	AstNode*	m_right_expr;
	SourceInfo	m_si_op;

	TypeId m_left_expr_tid;
	TypeId m_right_expr_tid;

	bool m_left_expr_is_tmp;
	bool m_right_expr_is_tmp;

	FnAddr m_fn_addr;
};
