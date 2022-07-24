#include <assert.h>

#include "type.h"
#include "type_int.h"
#include "type_mgr.h"
#include "type_type.h"

TypeMgr g_typemgr;

TypeMgr::TypeMgr() {
	set_primary_types();
}

void TypeMgr::set_primary_types() {
	{
		TypeInfo* ti_none = new TypeInfo();
		ti_none->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_none);
		assert(TYPE_ID_NONE==ti_none->GetTypeId());
	}
	{
		TypeInfo* ti_infer = new TypeInfo();
		ti_infer->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_infer);
		assert(TYPE_ID_INFER==ti_infer->GetTypeId());
	}
	{
		TypeInfoType* ti_type = new TypeInfoType();
		ti_type->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_type);
		assert(TYPE_ID_TYPE==ti_type->GetTypeId());
	}
	{
		TypeInfoInt* ti_int = new TypeInfoInt();
		ti_int->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_int);
		assert(TYPE_ID_INT==ti_int->GetTypeId());
	}
}

TypeId TypeMgr::allocate_typeid() {
	TypeId tid = (TypeId)m_typeinfos.size();
	return tid;
}
TypeInfo* TypeMgr::GetTypeInfo(TypeId tid) const {
	assert(tid >= 0);
	return m_typeinfos.at(tid);
}
const char* TypeMgr::GetTypeDesc(TypeId tid) const {
	return this->GetTypeInfo(tid)->GetDesc().c_str();
}
