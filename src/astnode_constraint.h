#pragma once

#include <locale>
#include <string>
#include <vector>

#include "astnode.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 类型约束. 对类型的限定条件的集合
 *
 * 示例:
 *		restriciton Foo[T,K]{
 * 		 	fn foo(another []T) K;
 * 		}
 *
 *		restriciton Add{
 * 		 	fn add(another []int) int;
 * 		}
 */
class AstNodeConstraint : public AstNode {
public:
	/*
	 * name 泛型约束名字
	 * generic_params 泛型约束的参数
	 * rules 规则
	 */
	AstNodeConstraint(std::string name, std::vector<std::string> generic_params, std::vector<ParserFnDeclare> rules);

	virtual VerifyContextResult Verify(VerifyContext& ctx, VerifyContextParam vparam) override;
	virtual Variable*			Execute(ExecuteContext& ctx) override;

	virtual AstNode*   DeepClone() override { return DeepCloneT(); }
	AstNodeConstraint* DeepCloneT();

	bool   HasGenericParam() const { return !m_generic_params.empty(); }
	TypeId Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params);

private:
	AstNodeConstraint(){}
private:
	std::string					 m_name;
	std::vector<std::string>	 m_generic_params;
	std::vector<ParserFnDeclare> m_rules;

	std::map<std::string, TypeId> m_instances;
};
