#pragma once

#include "astnode_blockstmt.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"
#include <vector>

/*
 * 函数: 包括普通函数, 方法
 */
class Function {
public:
	// 指向内置函数的指针
	typedef Variable* (*BuiltinCallback)(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args);

public:
	Function(TypeId tid, std::vector<std::string> params_name, AstNodeBlockStmt* body) {
		m_typeid		   = tid;
		m_thisobj		   = nullptr;
		m_builtin_callback = nullptr;
		m_params_name	   = params_name;
		m_body			   = body;
	}
	Function(TypeId tid, BuiltinCallback builtin_callback) {
		m_typeid		   = tid;
		m_thisobj		   = nullptr;
		m_builtin_callback = builtin_callback;
		m_body			   = nullptr;
	}

	TypeId				GetTypeId() const { return m_typeid; }
	void				SetThisObj(Variable* thisobj) { m_thisobj = thisobj; }
	VerifyContextResult Verify(VerifyContext& ctx);
	Variable*			Call(ExecuteContext& ctx, std::vector<Variable*> args);

	/*
	 * 检查函数的参数类型是否匹配
	 */
	bool   VerifyArgsType(std::vector<TypeId> args_type) const;
	TypeId GetReturnTypeId() const;

private:
	TypeId					 m_typeid;
	Variable*				 m_thisobj; // 所属对象. 普通函数则为null
	BuiltinCallback			 m_builtin_callback;
	std::vector<std::string> m_params_name;
	AstNodeBlockStmt*		 m_body;
};
