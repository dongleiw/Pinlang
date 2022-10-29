#pragma once

#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

class AstNode {
public:
	enum VerifyStatus {
		NOT_VERIFIED,
		IN_VERIFY,
		VERIFIED
	};

public:
	AstNode() : m_result_typeid(TYPE_ID_NONE), m_parent(nullptr), m_verify_status(NOT_VERIFIED) {
	}
	TypeId GetResultTypeId() { return m_result_typeid; }

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) = 0;
	virtual Variable*			Execute(ExecuteContext& ctx)						  = 0;

	void	 SetParent(AstNode* parent) { m_parent = parent; }
	AstNode* GetParent() { return m_parent; }

	virtual AstNode* DeepClone() = 0;

	bool IsInFor() const;
	bool IsVerified() const { return m_verify_status; }

protected:
	void verify_begin();
	void verify_end();

protected:
	TypeId		 m_result_typeid;
	AstNode*	 m_parent;
	VerifyStatus m_verify_status; // 是否已经verify
};

#define M_DEEP_CLONE(node, type) (node == nullptr ? nullptr : dynamic_cast<type>(node->DeepClone()))
