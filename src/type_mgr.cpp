#include <assert.h>

#include "type.h"
#include "type_int.h"
#include "type_mgr.h"
#include "log.h"
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
		assert(TYPE_ID_NONE == ti_none->GetTypeId());
	}
	{
		TypeInfo* ti_infer = new TypeInfo();
		ti_infer->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_infer);
		assert(TYPE_ID_INFER == ti_infer->GetTypeId());
	}
	{
		TypeInfoType* ti_type = new TypeInfoType();
		ti_type->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_type);
		assert(TYPE_ID_TYPE == ti_type->GetTypeId());
		m_typename_2_typeid["type"] = TYPE_ID_TYPE;
	}
	{
		TypeInfoInt* ti_int = new TypeInfoInt();
		ti_int->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_int);
		assert(TYPE_ID_INT == ti_int->GetTypeId());
		m_typename_2_typeid["int"] = TYPE_ID_INT;
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
std::string TypeMgr::GetTypeDesc(TypeId tid) const {
	return this->GetTypeInfo(tid)->GetDesc();
}
TypeId TypeMgr::GetTypeIdByName_or_unresolve(std::string type_name) {
	auto iter = m_typename_2_typeid.find(type_name);
	if (iter != m_typename_2_typeid.end()) {
		return iter->second;
	}
	TypeId new_tid				   = allocate_typeid();
	log_info("unresolved type name[%s] tid[%d]", type_name.c_str(), new_tid);
	m_typename_2_typeid[type_name] = new_tid;
	m_typeinfos.push_back(nullptr);
	return new_tid;
}
