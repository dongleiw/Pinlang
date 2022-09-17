#pragma once

#include <string>

#include "astnode.h"
#include "define.h"
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
		TYPE_KIND_FN,
	};

public:
	AstNodeType() {}
	void InitWithType();
	void InitWithIdentifier(std::string id);
	void InitWithFn(std::vector<ParserParameter> params, AstNodeType* return_type);

	// 生成类型id
	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override { return nullptr; };

	std::map<std::string, TypeId> InferType(TypeId target_tid) const;

	virtual AstNode*  DeepClone() override { return DeepCloneT(); }
	AstNodeType* DeepCloneT();
private:
	TypeKind					 m_type_kind;
	std::string					 m_id;
	std::vector<ParserParameter> m_fn_params;
	AstNodeType*				 m_fn_return_type;
};
