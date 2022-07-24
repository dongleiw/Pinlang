#pragma once

#include "type.h"
#include "verify_context.h"
#include "execute_context.h"
#include "variable.h"

class AstNode{
public:
	TypeId GetResultTypeId() { return m_result_typeid; }

	virtual VerifyContextResult Verify(VerifyContext& ctx)=0;
	virtual Variable* Execute(ExecuteContext& ctx)=0;

protected:
	TypeId m_result_typeid;
};
