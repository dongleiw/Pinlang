#pragma once

#include "stack.h"
#include "type.h"
#include "variable.h"
#include <optional>

class VerifyContextParam {
public:
	VerifyContextParam() {
		Clear();
	}
	VerifyContextParam(TypeId expect_result_tid) {
		Clear();
		m_expect_result_tid = expect_result_tid;
	}
	VerifyContextParam(TypeId expect_result_tid, TypeId expect_return_tid) {
		Clear();
		m_expect_result_tid = expect_result_tid;
		m_expect_return_tid = expect_return_tid;
	}

	void   SetResultTid(TypeId tid) { m_expect_result_tid = tid; }
	TypeId GetResultTid() const { return m_expect_result_tid; }

	void   SetReturnTid(TypeId tid) { m_expect_return_tid = tid; }
	TypeId GetReturnTid() const { return m_expect_return_tid; }

	void SetFnCallArgs(std::vector<TypeId> args_tid) {
		m_args_tid_is_set = true;
		m_args_tid		  = args_tid;
	}
	bool				HasFnCallArgs() const { return m_args_tid_is_set; }
	std::vector<TypeId> GetFnCallArgs() const { return m_args_tid; }

	void Clear() {
		m_expect_result_tid = TYPE_ID_INFER;
		m_expect_return_tid = TYPE_ID_NONE;
		m_args_tid_is_set	= false;
		m_args_tid.clear();
	}

private:
	TypeId				m_expect_result_tid;
	TypeId				m_expect_return_tid;
	bool				m_args_tid_is_set;
	std::vector<TypeId> m_args_tid;
};

class VerifyContextResult {
public:
	VerifyContextResult() {
		m_result_tid   = TYPE_ID_NONE;
		m_const_result = nullptr;
	}
	VerifyContextResult(TypeId result_tid) {
		m_result_tid   = result_tid;
		m_const_result = nullptr;
	}
	VerifyContextResult(TypeId result_tid, Variable* const_result) {
		m_result_tid   = result_tid;
		m_const_result = const_result;
	}

	TypeId GetResultTypeId() const { return m_result_tid; }
	void   SetResultTypeId(TypeId tid) { m_result_tid = tid; }

	void	  SetConstResult(Variable* const_result) { m_const_result = const_result; }
	Variable* GetConstResult() { return m_const_result; }

private:
	TypeId	  m_result_tid;
	Variable* m_const_result; // 如果是编译期常量, 放到这里
};

class VerifyContext {
public:
	VerifyContext();
	VerifyContextParam& GetParam() { return m_param; }
	VerifyContext&		SetParam(VerifyContextParam param) {
		 m_param = param;
		 return *this;
	}
	void   PushStack();
	void   PopSTack();
	Stack* GetCurStack() { return m_top_stack; }

private:
	void init_global_vt();

private:
	VerifyContextParam m_param;
	Stack*			   m_top_stack;
	VariableTable	   m_global_vt;
};
