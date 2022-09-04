#pragma once

#include "define.h"
#include "type.h"
#include <vector>

class ExecuteContext;
class Function;

class Variable {
public:
	Variable(TypeId tid) : m_tid(tid) {
	}
	Variable(int value);
	Variable(float value);
	Variable(std::string value);
	Variable(Function* fn);

	static Variable* CreateTypeVariable(TypeId tid);

	TypeId GetTypeId() const { return m_tid; }

	TypeId		GetValueTid() const { return m_value_tid; }
	int			GetValueInt() const { return m_value_int; }
	float		GetValueFloat() const { return m_value_float; }
	std::string GetValueStr() const { return m_value_str; }
	Function*	GetValueFunction() const { return m_value_fn; }

	bool IsConst() const { return m_is_const; }

	/*
	 * 找不到method, panic
	 */
	Variable* CallMethod(ExecuteContext& ctx, int method_idx, std::vector<Variable*> args);

	std::string ToString() const;

protected:
	TypeId		m_tid;
	bool		m_is_const;
	TypeId		m_value_tid;
	int			m_value_int;
	float		m_value_float;
	std::string m_value_str;
	Function*	m_value_fn;
};
