#pragma once

#include <utility>
#include <vector>

#include "astnode.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 一个block, 内部可以有任意数量的statment
 * 本身是statement而不是expression, 因此没有结果值
 * 如果该blockstatement在函数(方法)内, 则可以使用return返回值
 */
class AstNodeBlockStmt : public AstNode {
public:
	AstNodeBlockStmt(const std::vector<AstNode*>& stmts);
	AstNodeBlockStmt();

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(VM& vm, FnInstructionMaker& maker) override;

	void AddPreDefine(AstNodeBlockStmt& another);
	void AddChildStmt(AstNode* node);

	virtual AstNode*  DeepClone() override { return DeepCloneT(); }
	AstNodeBlockStmt* DeepCloneT();

	void SetGlobalBlock(bool b) { m_global_block = b; }

	void MergeAnother(AstNodeBlockStmt& another);

	void VerifyIdentfier(AstNode* cur_node, std::string id, VerifyContext& ctx, VerifyContextParam vparam);

private:
	std::vector<AstNode*>					   m_predefine_stmts;
	std::vector<AstNode*>					   m_stmts;
	bool									   m_global_block;
	std::vector<std::pair<AstNode*, AstNode*>> m_dependency; // 依赖关系. first依赖second
};
