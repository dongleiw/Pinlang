#pragma once

#include "define.h"
#include "fntable.h"
#include "stack.h"
#include "type.h"
#include "variable.h"
#include "variable_table.h"

#include <cassert>

class AstNodeBlockStmt;

class VerifyContextParam {
public:
	VerifyContextParam() {
		Clear();
	}
	VerifyContextParam& SetExpectResultTgid(TypeGroupId tgid) {
		m_expect_result_tgid = tgid;
		return *this;
	}
	TypeGroupId			GetExpectResultTgid() const { return m_expect_result_tgid; }
	VerifyContextParam& SetExpectResultTid(TypeId tid) {
		m_expect_result_tid = tid;
		return *this;
	}
	TypeId GetExpectResultTid() const { return m_expect_result_tid; }

	VerifyContextParam& SetExpectReturnTid(TypeId tid) {
		m_expect_return_tid = tid;
		m_expect_result_tgid = TYPE_GROUP_ID_FUNCTION;
		return *this;
	}
	TypeId GetExpectReturnTid() const { return m_expect_return_tid; }

	VerifyContextParam& SetExpectFnReturnTid(TypeId tid) {
		m_expect_fn_return_tid = tid;
		return *this;
	}
	TypeId GetExpectFnReturnTid() const { return m_expect_fn_return_tid; }

	VerifyContextParam& SetFnCallArgs(std::vector<TypeId> args_tid) {
		m_args_tid_is_set	 = true;
		m_args_tid			 = args_tid;
		m_expect_result_tgid = TYPE_GROUP_ID_FUNCTION;
		return *this;
	}
	bool				HasFnCallArgs() const { return m_args_tid_is_set; }
	std::vector<TypeId> GetFnCallArgs() const { return m_args_tid; }

	void Clear() {
		m_expect_result_tgid = TYPE_GROUP_ID_NONE;
		m_expect_result_tid	 = TYPE_ID_INFER;
		m_expect_return_tid	 = TYPE_ID_NONE;
		m_args_tid_is_set	 = false;
		m_args_tid.clear();
		m_expect_left_value = false;
	}

	VerifyContextParam& SetExepectLeftValue(bool b) {
		m_expect_left_value = b;
		return *this;
	}
	bool ExpectLeftValue() const { return m_expect_left_value; }

private:
	TypeGroupId			m_expect_result_tgid;
	TypeId				m_expect_result_tid;
	TypeId				m_expect_fn_return_tid; // 期望的函数返回类型. 仅在筛选函数时有效
	TypeId				m_expect_return_tid;
	bool				m_args_tid_is_set;
	std::vector<TypeId> m_args_tid;
	bool				m_expect_left_value;
};

class VerifyContextResult {
public:
	VerifyContextResult() {
		m_result_tid	   = TYPE_ID_NONE;
		m_const_result	   = nullptr;
		m_is_tmp		   = true;
		m_is_method		   = false;
		m_has_self_param = false;
		m_is_constructor   = false;
		m_obj_tid		   = TYPE_ID_NONE;
	}
	VerifyContextResult(TypeId result_tid) {
		m_result_tid	   = result_tid;
		m_const_result	   = nullptr;
		m_is_tmp		   = true;
		m_is_method		   = false;
		m_has_self_param = false;
		m_is_constructor   = false;
		m_obj_tid		   = TYPE_ID_NONE;
	}
	VerifyContextResult(TypeId result_tid, Variable* const_result) {
		m_result_tid	   = result_tid;
		m_const_result	   = const_result;
		m_is_tmp		   = true;
		m_is_method		   = false;
		m_has_self_param = false;
		m_is_constructor   = false;
		m_obj_tid		   = TYPE_ID_NONE;
	}

	TypeId GetResultTypeId() const { return m_result_tid; }
	void   SetResultTypeId(TypeId tid) { m_result_tid = tid; }

	bool	  IsConst() const { return m_const_result != nullptr; }
	void	  SetConstResult(Variable* const_result) { m_const_result = const_result; }
	Variable* GetConstResult() { return m_const_result; }

	VerifyContextResult& SetTmp(bool tmp) {
		m_is_tmp = tmp;
		return *this;
	}
	bool IsTmp() const { return m_is_tmp; }

	VerifyContextResult& SetHasSelfParam(bool b) {
		m_has_self_param = b;
		return *this;
	}
	bool HasSelfParam() const { return m_has_self_param; }

	VerifyContextResult& SetIsConstructor(bool b) {
		m_is_constructor = b;
		return *this;
	}
	bool IsConstructor() const { return m_is_constructor; }

	VerifyContextResult& SetIsMethod(bool b) {
		m_is_method = b;
		return *this;
	}
	bool IsMethod() const { return m_is_method; }

	VerifyContextResult& SetObjTid(TypeId obj_tid) {
		m_obj_tid = obj_tid;
		return *this;
	}
	TypeId GetObjTid() const { return m_obj_tid; }

private:
	TypeId	  m_result_tid;
	bool	  m_is_tmp;
	Variable* m_const_result; // 如果是编译期常量, 放到这里

	bool m_is_method;		 // 结果是一个函数, 该函数是一个方法
	bool m_has_self_param; // 结果是一个函数, 该函数第一个参数为隐藏的this参数
	bool m_is_constructor;	 // 结果是一个函数, 该函数是一个构造函数

	/*
	 * 对象的typeid
	 * 1. AstNodeIdentifier中发现当前id是一个构造函数, 会用该自动返回class的typeid
	 */
	TypeId m_obj_tid;
};

class VerifyContext {
public:
	VerifyContext(std::vector<AstNodeBlockStmt*> global_block_stmts);
	void		   PushStack();
	void		   PopStack();
	Stack*		   GetCurStack() { return m_top_stack; }
	VariableTable* GetGlobalVt() { return &m_global_vt; }
	FnTable&	   GetFnTable() { return m_fn_table; }
	void		   Verify();
	void		   VerifyGlobalIdentifier(const AstNode* cur_node, std::string id, VerifyContextParam vparam);

private:
	Stack*						   m_top_stack;
	VariableTable				   m_global_vt;
	std::vector<AstNodeBlockStmt*> m_global_block_stmts;
	FnTable						   m_fn_table;
};
