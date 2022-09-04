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
	 * 函数有重载时, fnname不唯一, 该函数根据函数参数类型生成后缀, 与fnname拼接得到唯一name
	 * uniq_fnname = fnname + uniq_fnname_suffix
	 *
	 * 1. 为何不加上函数返回类型?
	 *		不同的返回类型不构成重载, 因此这里不能加返回类型
	 *		如果将返回类型也纳入到重载的参数中, 会出现什么情况?
	*/
	static std::string GetUniqFnName(std::string fnname, std::vector<TypeId> concrete_generic_params, std::vector<TypeId> params_tid);
	static std::string GetUniqFnName(std::string fnname, std::vector<TypeId> params_tid);

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
	//void set_uniq_fn_name_suffix();

private:
	std::vector<TypeId> m_params;
	TypeId				m_return_tid;
	std::string			m_uniq_fnname_suffix;
};
