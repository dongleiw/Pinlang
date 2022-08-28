#pragma once

#include "type.h"
#include "type_fn.h"

#include <vector>
#include <map>

class TypeInfo;

class TypeMgr{
public:
	TypeMgr();

	/*
	 * 如果不存在, panic
	 */
	TypeInfo* GetTypeInfo(TypeId tid)const;
	std::string GetTypeName(TypeId tid)const;

	/*
	 * 根据类型名获取类型id. 没有找到则panic
	 */
	TypeId GetTypeIdByName(std::string type_name);

	bool HasTypeIdByName(std::string type_name);

	/*
	 * 根据类型名获取类型id. 没有找到则增加一个未解决类型
	 */
	TypeId GetTypeIdByName_or_unresolve(std::string type_name);

	/*
	 * 根据参数类型和返回类型获取函数类型
	 * 如果不存在, 增加一个
	 */
	TypeId GetOrAddTypeFn(std::vector<Parameter> params, TypeId return_tid);

	void InitTypes();
private:
	/*
	 * 增加类型
	 * 如果已经存在, 则panic
	 * @ti 必须分配在堆
	 */
	TypeId add_type(TypeInfo* heap_alloc_ti);
	TypeId allocate_typeid();

private:
	std::vector<TypeInfo*> m_typeinfos;
	/*
	 * 类型的名字到类型id的映射
	 * 某些类型名字可能指向的typeid对应的typeinfo是nullptr
	 * 类型名字: 
	 *		- 基础类型: int float str bool
	 *		- 函数: fn(int,int)int
	 *		- class类型: Person Student Book ...
	 */
	std::map<std::string, TypeId> m_typename_2_typeid;
};

extern TypeMgr g_typemgr;

#define GET_TYPENAME_C(tid) (g_typemgr.GetTypeName(tid).c_str())
#define GET_TYPENAME(tid) (g_typemgr.GetTypeName(tid))
