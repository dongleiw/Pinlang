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
 *		obj_expr.attr_name
 *			obj_expr: 
 *				类型如果是type(包裹的类型是T), 则实际访问的是T的属性
 *				类型如果是*T, 则自动解引用
 *				如果是其他类型(类型为F), 则访问的是F的属性
 *			attr_name: 属性名. 必须是一个identifier
 * 例子:
 *		2.tostring
 *		person.name
 *		Person.ConstructWithName("vigoss")
 */
class AstNodeAccessAttr : public AstNode {
public:
	AstNodeAccessAttr(AstNode* obj_expr, std::string attr_name);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	AstNodeAccessAttr* DeepCloneT();
	virtual AstNode*   DeepClone() override { return DeepCloneT(); }

	bool IsSimpleFn(VerifyContext& vctx);

private:
	AstNode*	m_obj_expr;
	std::string m_attr_name;

	bool				m_is_field;
	std::string			m_fnid;
	bool				m_is_obj_expr_verified;
	VerifyContextResult m_vr_obj_expr;
	bool				m_obj_is_type;

	TypeId m_obj_tid;
};
