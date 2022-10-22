#pragma once

#include "define.h"
#include "type.h"

/*
 * 元组类型
 */
class TypeInfoTuple : public TypeInfo {
public:
	// 内存结构
	struct MemStructure {
		uint64_t size; // 元素个数
		uint8_t* data; // 数据.  data length = size * sizeof(ArrayType)

		MemStructure() : size(0), data(nullptr) {
		}
	};
public:
	static std::string GetFieldName(int idx);
public:
	TypeInfoTuple(std::vector<TypeId> element_tids);
	void InitBuiltinMethods(VerifyContext& ctx) override;

	const std::vector<TypeId>& GetElementTids() const { return m_element_tids; }

private:
	std::string generate_name();
private:
	std::vector<TypeId> m_element_tids;
};
