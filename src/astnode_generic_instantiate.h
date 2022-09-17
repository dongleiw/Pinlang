#pragma once

#include <string>

#include "astnode.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "type_fn.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 表达式: 泛型实例化
 *		sum[int,float]
 */
class AstNodeGenericInstantiate : public AstNode {
public:
	AstNodeGenericInstantiate(std::string generic_name, std::vector<AstNodeType*> type_list);

	virtual VerifyContextResult Verify(VerifyContext& ctx) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode*		   DeepClone() override { return DeepCloneT(); }
	AstNodeGenericInstantiate* DeepCloneT();

private:
	AstNodeGenericInstantiate() {}

private:
	std::string				  m_generic_name;
	std::vector<AstNodeType*> m_type_list;

	std::string m_instance_name;
};
