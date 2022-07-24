#pragma once

#include "type.h"
class Variable {
  public:
	Variable(TypeId tid): m_tid(tid){
	}
	Variable(int value);

	TypeId GetTypeId() const { return m_tid; }
	int GetValueInt() const { return m_value_int; }

  protected:
	TypeId m_tid;
	int m_value_int;
};
