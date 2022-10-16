#pragma once

#include "define.h"
#include "type.h"

/*
 * 元组类型
 */
class TypeInfoTuple : public TypeInfo {
public:
	static std::string GetFieldName(int idx);
public:
	TypeInfoTuple(std::vector<TypeId> element_tids);
	void InitBuiltinMethods(VerifyContext& ctx) override;

	const std::vector<TypeId>& GetElementTids() const { return m_element_tids; }

private:
	std::string generate_name();
	void set_fields();
private:
	std::vector<TypeId> m_element_tids;
};
