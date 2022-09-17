#pragma once

#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

class AstNode {
public:
	AstNode() : m_result_typeid(TYPE_ID_NONE), m_parent(nullptr) {
	}
	TypeId GetResultTypeId() { return m_result_typeid; }

	virtual VerifyContextResult Verify(VerifyContext& ctx)	 = 0;
	virtual Variable*			Execute(ExecuteContext& ctx) = 0;

	void SetParent(AstNode* parent) { m_parent = parent; }
	const AstNode* GetParent() const { return m_parent; }

protected:
	TypeId	 m_result_typeid;
	AstNode* m_parent;
};
