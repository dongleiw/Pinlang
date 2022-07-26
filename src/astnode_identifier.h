#pragma once

#include <string>

#include "astnode.h"
#include "fntable.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

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
	AstNodeIdentifier(std::string id) : m_id(id), m_is_complex_fn(false) {
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vr_param) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode*   DeepClone() override { return DeepCloneT(); }
	AstNodeIdentifier* DeepCloneT();

private:
	std::string m_id;

	FnAddr m_fn_addr; // 如果identier为函数, 则保存函数地址
	bool	m_is_complex_fn;
};
