#pragma once

#include <string>

#include "astnode.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 类型. 在parser阶段, 信息不全导致不易生成typeid. 因此延期到verify阶段
 *
 */
class AstNodeType : public AstNode {
public:
	enum TypeKind {
		TYPE_KIND_TYPE,
		TYPE_KIND_IDENTIFIER,
	};

public:
	AstNodeType() {}
	void InitWithType();
	void InitWithIdentifier(std::string id);

	// 生成类型id
	virtual VerifyContextResult Verify(VerifyContext& ctx) override;
	virtual Variable* Execute(ExecuteContext& ctx) override { return nullptr; };

	std::map<std::string, TypeId> InferType(TypeId target_tid) const;

private:
	TypeKind m_type_kind;
	std::string m_id;
};
