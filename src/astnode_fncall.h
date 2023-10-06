#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 被调用的函数有几种情况:
 *		1. identifier指定的普通函数
 *			add(1,2)
 *			直接校验所有参数
 *		2. Type指定的同名构造函数
 *			Foo(1, 2)
 *			使用Foo的同名构造函数. 等价于: Foo.Foo(1,2)
 *			构造函数有隐藏的this参数, 校验参数时需要忽略. 该参数由AstNodeFnCall创建
 *		3. 显式指定的构造函数
 *			Foo.Create(1,2)
 *			显式指定构造函数`Create`. 
 *		3. 对象的非静态方法
 *			f.GetId(1, 2)
 *			非静态方法有隐藏的this参数, 校验参数时需要忽略.
 *		4. 函数指针
 *			可能指向普通函数
 *				var pf = add;
 *				pf(1, 2);
 *			也可能指向方法
 *				var pf = f.GetId;
 *				pf(f, 1, 2); // 需要显式指定this参数
 *			直接校验所有参数
 */
class AstNodeFnCall : public AstNode {
public:
	AstNodeFnCall(AstNode* fn_expr, std::vector<AstNode*> args);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;
	virtual CompileResult		Compile(CompileContext& cctx) override;

	virtual AstNode* DeepClone() override { return DeepCloneT(); }
	AstNodeFnCall*	 DeepCloneT();

private:
	AstNodeFnCall() {}

private:
	AstNode*			  m_fn_expr;
	std::vector<AstNode*> m_args;

	TypeId				m_fn_tid;
	VerifyContextResult m_vr_fn;
};
