#pragma once

#include "type.h"
#include "vector"

class TypeInfo;

class TypeMgr{
public:
	TypeMgr();

	TypeInfo* GetTypeInfo(TypeId tid)const;
	const char* GetTypeDesc(TypeId tid)const;

private:
	TypeId allocate_typeid();
	void set_primary_types();

private:
	std::vector<TypeInfo*> m_typeinfos;
};

extern TypeMgr g_typemgr;
