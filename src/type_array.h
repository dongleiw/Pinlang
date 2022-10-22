#pragma once

#include "type.h"

/*
 * 数组类型
 */
class TypeInfoArray : public TypeInfo {
public:
	// 内存结构
	struct MemStructure {
		uint64_t size; // 元素个数
		uint8_t* data; // 数据.  data length = size * sizeof(ArrayType)

		MemStructure() : size(0), data(nullptr) {
		}
	};

public:
	TypeInfoArray(TypeId element_tid);
	void InitBuiltinMethods(VerifyContext& ctx) override;

	TypeId GetElementType() const { return m_element_tid; }

private:
	TypeId m_element_tid;
};
