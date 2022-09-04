#pragma once

#include "define.h"
#include "type.h"

/*
 * 类型约束. 对类型的限定条件的集合
 */
class TypeInfoGenericRestriction : public TypeInfo {
public:
	/*
	 * 泛型参数
	 */
	struct GenericParam {
		std::string name;
		std::string default_value_name; // 默认值
		TypeId		real_tid;
		GenericParam() : real_tid(TYPE_ID_NONE) {}
		GenericParam(std::string name, std::string default_value_name) : name(name), default_value_name(default_value_name), real_tid(TYPE_ID_NONE) {
		}
	};
	/*
	 * 函数泛型结构
	 */
	struct GenericFunction {
		std::vector<std::string> params_type;
		std::string				 return_type;
	};
	/* 
	 * 一条约束
	 * 方法名 + 方法类型
	*/
	struct Rule {
		std::string		fn_name;
		GenericFunction function;
	};

public:
	TypeInfoGenericRestriction(std::string name, std::vector<GenericParam> generic_params, std::vector<Rule> rules) {
		m_name			 = name;
		m_generic_params = generic_params;
		m_generic_rules	 = rules;
		m_typegroup_id	 = TYPE_GROUP_ID_GENERIC_RESTRICTION;
	}

	/* 
	 * 根据输入类型, 生成类型确定的restriction实例
	 * 例如如下restriction泛型:
	 *		restriction Add<T=Self, Output=T>{ 
	 * 		 	fn add(another T) Output; 
	 * 		}
	 * 		给定参数: T=int的情况下. 推导得到
	 *		restriction Add_int{ 
	 * 		 	fn add(another int) int; 
	 * 		}
	 *
	 * 		实例化后的restriction名字为: Add<int,int>
	 */
	TypeId Instantiate(std::map<std::string, TypeId> params) const;

public: // test
	/*
	 * restriction Add<AnotherT, OutputT=AnotherT>{
	 *		fn add(anotherT Another) OutputT;
	 * }
	 */
	static TypeInfoGenericRestriction* create_restriction_add();
	/*
	 * restriction Sub<AnotherT, OutputT=AnotherT>{
	 *		fn sub(anotherT Another) OutputT;
	 * }
	 */
	static TypeInfoGenericRestriction* create_restriction_sub();
	/*
	 * restriction Mul<AnotherT, OutputT=AnotherT>{
	 *		fn mul(anotherT Another) OutputT;
	 * }
	 */
	static TypeInfoGenericRestriction* create_restriction_mul();
	/*
	 * restriction Div<AnotherT, OutputT=AnotherT>{
	 *		fn div(anotherT Another) OutputT;
	 * }
	 */
	static TypeInfoGenericRestriction* create_restriction_div();
	static TypeInfoGenericRestriction* create_restriction_mod();

private:
	std::vector<GenericParam> m_generic_params;
	std::vector<Rule>		  m_generic_rules;
};
