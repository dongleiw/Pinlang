#pragma once

#include "type.h"

class VerifyContextParam{
};

class VerifyContextResult{
public:
	VerifyContextResult(){
		m_result_tid = TYPE_ID_NONE;
	}
	VerifyContextResult(TypeId result_tid){
		m_result_tid = result_tid;
	}

	TypeId GetTypeId() const {  return m_result_tid; }
	void SetTypeId(TypeId tid) {  m_result_tid=tid; }
private:
	TypeId m_result_tid;
};

class VerifyContext{
};
