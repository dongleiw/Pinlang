#pragma once

#include "type.h"

/*
 * 数组类型
 */
class TypeInfoArray : public TypeInfo {
public:
	TypeInfoArray(TypeId element_tid);
	void InitBuiltinMethods(VerifyContext& ctx) override;

	TypeId GetElementType() const { return m_element_tid; }

private:
	TypeId m_element_tid;
};
