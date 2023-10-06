#pragma once

#include "compile_context.h"
#include "execute_context.h"
#include "instruction.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/Value.h>

enum AstType {
	NONE	   = 0,
	IDENTIFIER = 1,
	FN_CALL	   = 2,
};

class AstNode {
public:
	enum VerifyStatus {
		NOT_VERIFIED,
		IN_VERIFY,
		VERIFIED
	};

public:
	AstNode() : m_ast_type(AstType::NONE), m_result_typeid(TYPE_ID_NONE), m_parent(nullptr), m_verify_status(NOT_VERIFIED), m_is_exit_node(false), m_compile_to_left_value(false) {
	}
	virtual ~AstNode() {}
	TypeId GetResultTypeId() { return m_result_typeid; }

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) = 0;
	virtual Variable*			Execute(ExecuteContext& ctx);
	virtual CompileResult		Compile(CompileContext& cctx);
	virtual void				ClearVerifyState();

	void	 SetParent(AstNode* parent) { m_parent = parent; }
	AstNode* GetParent() { return m_parent; }

	virtual AstNode* DeepClone() = 0;
	void			 Copy(AstNode& from);

	bool		 IsInFor() const;
	bool		 IsInFn() const;
	bool		 IsVerified() const { return m_verify_status; }
	virtual bool IsLiteral() const { return false; }

	bool IsExitNode() const { return m_is_exit_node; }

	bool		 IsCompileToLeftValue() const { return m_compile_to_left_value; }
	virtual void SetCompileToLeftValue() { m_compile_to_left_value = true; }
	virtual void SetCompileToRightValue() { m_compile_to_left_value = false; }

	AstType GetAstType() const { return m_ast_type; }

protected:
	void verify_begin();
	void verify_end();

protected:
	AstType		 m_ast_type;
	TypeId		 m_result_typeid;
	AstNode*	 m_parent;
	VerifyStatus m_verify_status; // 是否已经verify
	bool		 m_is_exit_node;  // 是否是exitnode
	bool		 m_compile_to_left_value;
};

#define M_DEEP_CLONE(node, type) (node == nullptr ? nullptr : dynamic_cast<type>(node->DeepClone()))
#define VERIFY_BEGIN \
	verify_begin();  \
	Defer __defer_var__([this]() { this->verify_end(); });
