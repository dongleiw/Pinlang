#pragma once

#include <string>

#include "astnode.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 访问对象的属性
 *		2.tostring
 *		person.name
 */
class AstNodeAccessAttr : public AstNode {
public:
	AstNodeAccessAttr(AstNode* obj_expr, std::string attr_name);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	AstNodeAccessAttr* DeepCloneT();
	virtual AstNode*   DeepClone() override { return DeepCloneT(); }

private:
	AstNode*	m_obj_expr;
	std::string m_attr_name;

	bool		m_is_field;
	FnAddr		m_fn_addr;
	std::string m_fnid;

	TypeId m_obj_tid;
};
