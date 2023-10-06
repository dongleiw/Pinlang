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

/*
 * 标识符(变量名, 函数名, 类型名)
 *
 * 例子:
 *		id = xxx; // 被赋值
 *		xxx = id; // 获取值
 *		id+1; // 获取值
 */
class AstNodeIdentifier : public AstNode {
public:
	AstNodeIdentifier(std::string id);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vr_param) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;
	virtual void				ClearVerifyState() override;

	virtual AstNode*   DeepClone() override { return DeepCloneT(); }
	AstNodeIdentifier* DeepCloneT();

	bool IsSimpleFn(VerifyContext& vctx);

	std::string GetId() const { return m_id; }

	// 来自access_attr语法
	VerifyContextResult Verify_as_tid(VerifyContext& ctx, bool &is_type);

private:
	std::string m_id;

	std::string m_fn_id; // 如果identifier为静态函数, 保存函数的实际的名字
	bool		m_is_complex_fn;
};
