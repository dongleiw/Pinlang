#pragma once

#include "type.h"
#include <bits/stdint-uintn.h>

/*
 * 基础的str类型
 * struct{
 *		const char* ptr;
 *		size_t size;
 * }
 */
class TypeInfoStr : public TypeInfo {
public:
	// 内存结构
	struct MemStructure {
		uint64_t size; // 长度
		uint8_t* data; // 数据. 自动增加NULL结尾

		MemStructure() : size(0), data(nullptr) {
		}
	};

public:
	TypeInfoStr();

	void InitBuiltinMethods(VerifyContext& ctx) override;

private:
};
