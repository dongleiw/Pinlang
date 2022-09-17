#pragma once

#include "define.h"
#include "type.h"
#include "type_generic_type.h"

/*
 * 类型约束. 对类型的限定条件的集合
 */
class TypeInfoRestriction : public TypeInfo {
public:
	/* 
	 * 一条约束
	 * 方法名 + 方法类型
	*/
	struct Rule {
		std::string fn_name;
		TypeId		fn_tid;
	};

public:
	TypeInfoRestriction(std::string name, std::vector<Rule> rules) {
		m_name		   = name;
		m_rules		   = rules;
		m_typegroup_id = TYPE_GROUP_ID_RESTRICTION;
	}

	// 生成一个虚拟的泛型类型, 满足该约束
	void FillVirtualType(TypeInfoGenericType& ti) const;

private:
	std::vector<Rule> m_rules;
};
