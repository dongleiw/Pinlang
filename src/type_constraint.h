#pragma once

#include "define.h"
#include "type.h"
#include "type_virtual_gtype.h"
#include "verify_context.h"

/*
 * 类型约束. 对类型的限定条件的集合
 */
class TypeInfoConstraint : public TypeInfo {
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
	TypeInfoConstraint(std::string constraint_name, std::string constraint_instance_name, std::vector<Rule> rules) {
		m_constraint_name = constraint_name;
		m_original_name	  = constraint_instance_name;
		m_rules			  = rules;
		m_typegroup_id	  = TYPE_GROUP_ID_CONSTRAINT;
	}

	// 生成一个虚拟的泛型类型, 满足该约束
	//void FillVirtualType(VerifyContext& ctx, TypeInfoVirtualGType& ti) const;

	std::string GetConstraintName() const { return m_constraint_name; }

private:
	std::string		  m_constraint_name;
	std::vector<Rule> m_rules;
};
