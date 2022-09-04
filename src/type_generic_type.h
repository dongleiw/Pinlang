#pragma once

#include "type.h"

/*
 * 泛型类型
 * 本身无法执行, 仅用于校验和占位
 */
class TypeInfoGenericType : public TypeInfo {
public:
	TypeInfoGenericType(std::string name);

	/*
	 * 创建新类型, 将类型中的泛型id替换为实际类型id
	 * 如果本身不是泛型, 返回nullptr
	 * 如果对应关系不全, 则panic
	 */
	virtual TypeInfo* ToConcreteType(std::map<TypeId, TypeId> gtid_2_ctid) const override;
private:
};
