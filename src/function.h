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
		m_typeid			 = tid;
		m_obj_tid			 = obj_tid;
		m_builtin_callback	 = nullptr;
		m_gparams			 = gparams;
		m_params_name		 = params_name;
		m_body				 = body;
		m_dynlib_instance_id = -1;
		m_dynlib_fn			 = nullptr;
	}
	Function(TypeId tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, BuiltinFnCallback builtin_callback) {
		m_typeid			 = tid;
		m_obj_tid			 = obj_tid;
		m_builtin_callback	 = builtin_callback;
		m_gparams			 = gparams;
		m_params_name		 = params_name;
		m_body				 = nullptr;
		m_dynlib_instance_id = -1;
		m_dynlib_fn			 = nullptr;
	}
	Function(TypeId tid, TypeId obj_tid, std::vector<ConcreteGParam> gparams, std::vector<std::string> params_name, BuiltinFnCallback builtin_callback, int dylib_instance_id, void* dylib_fn) {
		m_typeid			 = tid;
		m_obj_tid			 = obj_tid;
		m_builtin_callback	 = builtin_callback;
		m_gparams			 = gparams;
		m_params_name		 = params_name;
		m_body				 = nullptr;
		m_dynlib_instance_id = dylib_instance_id;
		m_dynlib_fn			 = dylib_fn;
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

	int	  GetDynLibInstanceId() const { return m_dynlib_instance_id; }
	void* GetDynLibFn() const { return m_dynlib_fn; }

private:
	TypeId						m_typeid;
	TypeId						m_obj_tid; // 如果是方法, 则为对象的类型. 否则是TYPE_ID_NONE
	std::vector<ConcreteGParam> m_gparams;
	std::vector<std::string>	m_params_name;

	// 代码定义函数
	AstNodeBlockStmt* m_body;
	// 内置函数
	BuiltinFnCallback m_builtin_callback;

	// 动态库加载得到的函数
	int	  m_dynlib_instance_id;
	void* m_dynlib_fn;
};
