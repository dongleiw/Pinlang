#pragma once

#include "execute_context.h"
#include "instruction.h"
#include "llvm_ir.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/Value.h>

class AstNode {
public:
	enum VerifyStatus {
		NOT_VERIFIED,
		IN_VERIFY,
		VERIFIED
	};

public:
	AstNode() : m_result_typeid(TYPE_ID_NONE), m_parent(nullptr), m_verify_status(NOT_VERIFIED), m_is_exit_node(false) {
	}
	TypeId GetResultTypeId() { return m_result_typeid; }

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) = 0;
	virtual Variable*			Execute(ExecuteContext& ctx)						  = 0;
	virtual llvm::Value*		Compile(LLVMIR& llvm_ir);

	void	 SetParent(AstNode* parent) { m_parent = parent; }
	AstNode* GetParent() { return m_parent; }

	virtual AstNode* DeepClone() = 0;

	bool IsInFor() const;
	bool IsVerified() const { return m_verify_status; }

	bool IsExitNode() const { return m_is_exit_node; }

protected:
	void verify_begin();
	void verify_end();

protected:
	TypeId		 m_result_typeid;
	AstNode*	 m_parent;
	VerifyStatus m_verify_status; // 是否已经verify
	bool		 m_is_exit_node;  // 是否是exitnode
};

#define M_DEEP_CLONE(node, type) (node == nullptr ? nullptr : dynamic_cast<type>(node->DeepClone()))
