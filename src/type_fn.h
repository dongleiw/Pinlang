#pragma once

#include "type.h"
#include <vector>

struct Parameter {
	TypeId arg_tid;
};

/*
 * 函数类型
 */
class TypeInfoFn : public TypeInfo {
public:
	TypeInfoFn(std::vector<Parameter> params, TypeId return_tid);

	TypeId GetReturnTypeId()const { return m_return_tid; }
	bool VerifyArgsType(std::vector<TypeId> args_type);
private:
	void set_name();
private:
	std::vector<Parameter> m_params;
	TypeId m_return_tid;
};
