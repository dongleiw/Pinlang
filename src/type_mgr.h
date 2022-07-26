#pragma once

#include "type.h"

#include <vector>
#include <map>

class TypeInfo;

class TypeMgr{
public:
	TypeMgr();

	TypeInfo* GetTypeInfo(TypeId tid)const;
	std::string GetTypeDesc(TypeId tid)const;

	TypeId GetTypeIdByName_or_unresolve(std::string type_name);

	//void AddTypedef(std::string type_name, TypeId); 

private:
	TypeId allocate_typeid();
	void set_primary_types();

private:
	std::vector<TypeInfo*> m_typeinfos;
	/*
	 * 类型的名字到类型id的映射
	 * 某些类型名字可能指向TYPE_ID_
	 */
	std::map<std::string, TypeId> m_typename_2_typeid;
};

extern TypeMgr g_typemgr;

#define get_typedesc(tid) (g_typemgr.GetTypeDesc(tid).c_str())
