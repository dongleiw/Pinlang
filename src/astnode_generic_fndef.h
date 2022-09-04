#pragma once

#include <string>

#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_type.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "type_fn.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 泛型函数定义
 * fn sum[T Add[T]](a T, b T) T{ 
 *		return a+b; 
 * }
 */
class AstNodeGenericFnDef : public AstNode {
public:
	AstNodeGenericFnDef(std::string fn_name, std::vector<ParserGenericParam> generic_params, std::vector<ParserParameter> params, AstNodeType* return_type, AstNodeBlockStmt* body);

	virtual VerifyContextResult Verify(VerifyContext& ctx) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	/*
	 * 根据参数和返回值来推导出泛型参数, 然后实例化, 然后添加到该泛型函数定义的相同block内
	 */
	std::string Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params_tid, TypeId concrete_return_tid);

private:
	/*
	 * 根据实际的泛型类型实例化
	 */
	std::string instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_generic_params);

	void add_instance_to_vt(VerifyContext& ctx, std::string name, Function* fn) const;

	void verify_restriction(VerifyContext& ctx, std::vector<TypeId> concrete_generic_params)const;

private:
	std::string						m_fnname;
	std::vector<ParserGenericParam> m_generic_params;
	std::vector<ParserParameter>	m_params;
	AstNodeType*					m_return_type;
	AstNodeBlockStmt*				m_body;

	std::map<std::string, Function*> m_instances;
};
