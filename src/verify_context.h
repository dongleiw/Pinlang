#pragma once

#include "stack.h"
#include "type.h"

class VerifyContextParam {
public:
	VerifyContextParam() : m_expect_result_tid(TYPE_ID_NONE) {}
	VerifyContextParam(TypeId expect_result_tid) : m_expect_result_tid(expect_result_tid) {}

private:
	TypeId m_expect_result_tid;
};

class VerifyContextResult {
public:
	VerifyContextResult() { m_result_tid = TYPE_ID_NONE; }
	VerifyContextResult(TypeId result_tid) { m_result_tid = result_tid; }

	TypeId GetResultTypeId() const { return m_result_tid; }
	void   SetResultTypeId(TypeId tid) { m_result_tid = tid; }

private:
	TypeId m_result_tid;
};

class VerifyContext {
public:
	VerifyContext();
	VerifyContextParam GetParam() { return m_param; }
	VerifyContext&	   SetParam(VerifyContextParam param) {
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
