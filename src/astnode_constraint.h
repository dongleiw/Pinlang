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
 *		constraint Foo[T,K]{
 * 		 	fn foo(another []T) K;
 * 		}
 *
 *		constraint Add{
 * 		 	fn add(another []int) int;
 * 		}
 *
 * 	如果是泛型约束, 需要进行实例化
 * 	方法可以接受*self, 也可以是静态方法
 */
class AstNodeConstraint : public AstNode {
public:
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
	ConstraintInstance Instantiate(VerifyContext& ctx, std::vector<TypeId> concrete_params, TypeId obj_tid);

	std::string GetName() const { return m_name; }

private:
	AstNodeConstraint() {}

private:
	std::string					 m_name;
	std::vector<std::string>	 m_generic_params;
	std::vector<ParserFnDeclare> m_rules;

	//std::map<std::string, TypeId> m_instances;
};
