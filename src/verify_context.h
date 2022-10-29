#pragma once

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
	VerifyContextParam& SetResultTid(TypeId tid) {
		m_expect_result_tid = tid;
		return *this;
	}
	TypeId GetResultTid() const { return m_expect_result_tid; }

	VerifyContextParam& SetReturnTid(TypeId tid) {
		m_expect_return_tid = tid;
		return *this;
	}
	TypeId GetReturnTid() const { return m_expect_return_tid; }

	VerifyContextParam& SetFnCallArgs(std::vector<TypeId> args_tid) {
		m_args_tid_is_set = true;
		m_args_tid		  = args_tid;
		return *this;
	}
	bool				HasFnCallArgs() const { return m_args_tid_is_set; }
	std::vector<TypeId> GetFnCallArgs() const { return m_args_tid; }

	void Clear() {
		m_expect_result_tid = TYPE_ID_INFER;
		m_expect_return_tid = TYPE_ID_NONE;
		m_args_tid_is_set	= false;
		m_args_tid.clear();
	}

	VerifyContextParam& SetExepectLeftValue(bool b) {
		m_expect_left_value = b;
		return *this;
	}
	bool ExpectLeftValue() const { return m_expect_left_value; }

private:
	TypeId				m_expect_result_tid;
	TypeId				m_expect_return_tid;
	bool				m_args_tid_is_set;
	std::vector<TypeId> m_args_tid;
	bool				m_expect_left_value;
};

class VerifyContextResult {
public:
	VerifyContextResult() {
		m_result_tid   = TYPE_ID_NONE;
		m_const_result = nullptr;
		m_is_tmp	   = true;
	}
	VerifyContextResult(TypeId result_tid) {
		m_result_tid   = result_tid;
		m_const_result = nullptr;
		m_is_tmp	   = true;
	}
	VerifyContextResult(TypeId result_tid, Variable* const_result) {
		m_result_tid   = result_tid;
		m_const_result = const_result;
		m_is_tmp	   = true;
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

private:
	TypeId	  m_result_tid;
	bool	  m_is_tmp;
	Variable* m_const_result; // 如果是编译期常量, 放到这里
};

class VerifyContext {
public:
	VerifyContext(AstNodeBlockStmt* global_block);
	void			  PushStack();
	void			  PopSTack();
	Stack*			  GetCurStack() { return m_top_stack; }
	VariableTable*	  GetGlobalVt() { return &m_global_vt; }
	AstNodeBlockStmt* GetGlobalBlock() { return m_global_block; }

private:
	Stack*			  m_top_stack;
	VariableTable	  m_global_vt;
	AstNodeBlockStmt* m_global_block;
};
