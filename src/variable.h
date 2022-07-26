#pragma once

#include "type.h"
class Variable {
public:
	Variable(TypeId tid) : m_tid(tid) {
	}
	Variable(int value);

	static Variable* CreateTypeVariable(TypeId tid);

	TypeId GetTypeId() const { return m_tid; }
	int	   GetValueInt() const { return m_value_int; }
	bool   IsConst() const { return m_is_const; }

protected:
	TypeId m_tid;
	bool   m_is_const;
	TypeId m_value_tid;
	int	   m_value_int;
};
