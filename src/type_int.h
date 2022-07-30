#pragma once

#include "type.h"

/*
 * 基础的int类型
 * 不同大小的integer以后再支持吧, 目前还用不到
 */
class TypeInfoInt : public TypeInfo {
public:
	TypeInfoInt();
	void InitBuiltinMethods() override;

private:
};
