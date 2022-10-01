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
	Function(TypeId tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, AstNodeBlockStmt* body) {
		m_typeid		   = tid;
		m_obj_tid		   = obj_tid;
		m_builtin_callback = nullptr;
		m_gparams		   = gparams;
		m_params_name	   = params_name;
		m_body			   = body;
	}
	Function(TypeId tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, BuiltinFnCallback builtin_callback) {
		m_typeid		   = tid;
		m_obj_tid		   = obj_tid;
		m_gparams		   = gparams;
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

	void SetObjTypeId(TypeId obj_tid) { m_obj_tid = obj_tid; }

private:
	TypeId						m_typeid;
	TypeId						m_obj_tid; // 如果是方法, 则为对象的类型. 否则是TYPE_ID_NONE
	BuiltinFnCallback			m_builtin_callback;
	std::vector<ConcreteGParam> m_gparams;
	std::vector<std::string>	m_params_name;
	AstNodeBlockStmt*			m_body;
};
