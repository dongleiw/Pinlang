#pragma once

#include "astnode_blockstmt.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "verify_context.h"
#include <vector>

class Variable;
/*
 * 函数逻辑: 包括普通函数, 方法
 */
class Function {
public:
	Function(TypeId tid, std::vector<std::string> params_name, AstNodeBlockStmt* body) {
		m_typeid		   = tid;
		m_builtin_callback = nullptr;
		m_params_name	   = params_name;
		m_body			   = body;
	}
	Function(TypeId tid, BuiltinFnCallback builtin_callback) {
		m_typeid		   = tid;
		m_builtin_callback = builtin_callback;
		m_body			   = nullptr;
	}

	TypeId				GetTypeId() const { return m_typeid; }
	VerifyContextResult Verify(VerifyContext& ctx);
	Variable*			Call(ExecuteContext& ctx, Variable* obj, std::vector<Variable*> args);

	/*
	 * 检查函数的参数类型是否匹配
	 */
	bool   VerifyArgsType(std::vector<TypeId> args_type) const;
	TypeId GetReturnTypeId() const;

private:
	TypeId					 m_typeid;
	BuiltinFnCallback		 m_builtin_callback;
	std::vector<std::string> m_params_name;
	AstNodeBlockStmt*		 m_body;
};
