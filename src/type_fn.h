#pragma once

#include "define.h"
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
	/* 
	 * 函数名可以对应多个实现. 该函数用于给每一个实现生成唯一名字
	 *	= fnname + 泛参信息 + 参数类型信息 + 返回类型信息
	*/
	static std::string GetUniqFnName(std::string fnname, std::vector<TypeId> concrete_generic_params, std::vector<TypeId> params_tid, TypeId return_tid);
	static std::string GetUniqFnName(std::string fnname, std::vector<TypeId> params_tid, TypeId return_tid);

public:
	TypeInfoFn(std::vector<TypeId> params, TypeId return_tid);

	TypeId GetReturnTypeId() const { return m_return_tid; }
	bool   VerifyArgsType(std::vector<TypeId> args_type);

	size_t GetParamNum() const { return m_params.size(); }
	TypeId GetParamType(size_t idx) const { return m_params.at(idx); }

	bool IsArgsTypeEqual(const TypeInfoFn& another) const;
	bool IsArgsTypeEqual(std::vector<TypeId> args_tid) const;

	std::vector<TypeId> GetParmsTid() const;

private:
	void set_name();

private:
	std::vector<TypeId> m_params;
	TypeId				m_return_tid;
};
