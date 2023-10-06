#pragma once

#include "type.h"
#include <assert.h>

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
	TypeInfoArray(TypeId element_tid, uint64_t size);
	void				InitBuiltinMethods(VerifyContext& ctx) override;
	virtual llvm::Type* GetLLVMIRType(CompileContext& cctx) override;
	virtual void		ConstructDefault(CompileContext& cctx, llvm::Value* obj) override;

	TypeId	 GetElementType() const { return m_element_tid; }
	bool	 IsStaticSize() const { return m_static_size > 0; }
	uint64_t GetStaticSize() const { return m_static_size; }

private:
	TypeId	 m_element_tid;
	uint64_t m_static_size; // 如果是编译期固定的数组, 该字段存放数组的元素个数, 否则为0
};
