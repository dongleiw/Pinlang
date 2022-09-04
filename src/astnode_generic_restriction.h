#pragma once

#include <string>
#include <vector>

#include "astnode.h"
#include "define.h"
#include "execute_context.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

/*
 * 泛型约束: 约束的条件中包括一些泛型.
 * 例如以下泛型约束
 *		restriciton Foo[T,K]{
 * 		 	fn foo(another []T) K;
 * 		}
 *
 * 而Add[int,int]则得到
 *		restriciton Add{
 * 		 	fn add(another []int) int;
 * 		}
 *
 */
class AstNodeGenericRestriction : public AstNode {
public:
	/*
	 * name 泛型约束名字
	 * generic_params 泛型约束的参数
	 * rules 规则
	 */
	AstNodeGenericRestriction(std::string name, std::vector<std::string> generic_params, std::vector<ParserFnDeclare> rules) {
		m_name			 = name;
		m_generic_params = generic_params;
		m_rules			 = rules;
	}

	virtual VerifyContextResult Verify(VerifyContext& ctx);
	virtual Variable*			Execute(ExecuteContext& ctx);

private:
	std::string					 m_name;
	std::vector<std::string>	 m_generic_params;
	std::vector<ParserFnDeclare> m_rules;
};
