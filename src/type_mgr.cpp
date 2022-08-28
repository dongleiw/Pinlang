#include <assert.h>

#include "define.h"
#include "log.h"
#include "type.h"
#include "type_float.h"
#include "type_fn.h"
#include "type_int.h"
#include "type_mgr.h"
#include "type_str.h"
#include "type_type.h"

TypeMgr g_typemgr;

TypeMgr::TypeMgr() {
}

void TypeMgr::InitTypes() {
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
	{
		TypeInfoFloat* ti_float = new TypeInfoFloat();
		ti_float->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_float);
		assert(TYPE_ID_FLOAT == ti_float->GetTypeId());
		m_typename_2_typeid["float"] = TYPE_ID_FLOAT;
	}
	{
		TypeInfoStr* ti_str = new TypeInfoStr();
		ti_str->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_str);
		assert(TYPE_ID_STR == ti_str->GetTypeId());
		m_typename_2_typeid["str"] = TYPE_ID_STR;
	}

	m_typeinfos.at(TYPE_ID_TYPE)->InitBuiltinMethods();
	m_typeinfos.at(TYPE_ID_INT)->InitBuiltinMethods();
	m_typeinfos.at(TYPE_ID_FLOAT)->InitBuiltinMethods();
	m_typeinfos.at(TYPE_ID_STR)->InitBuiltinMethods();
}

TypeId TypeMgr::allocate_typeid() {
	TypeId tid = (TypeId)m_typeinfos.size();
	return tid;
}
TypeInfo* TypeMgr::GetTypeInfo(TypeId tid) const {
	assert(tid >= 0);
	return m_typeinfos.at(tid);
}
std::string TypeMgr::GetTypeName(TypeId tid) const {
	return this->GetTypeInfo(tid)->GetName();
}
TypeId TypeMgr::GetTypeIdByName(std::string type_name) {
	auto iter = m_typename_2_typeid.find(type_name);
	if (iter != m_typename_2_typeid.end()) {
		return iter->second;
	}
	panicf("typename[%s] not exists", type_name.c_str());
}
bool TypeMgr::HasTypeIdByName(std::string type_name) {
	return m_typename_2_typeid.end() != m_typename_2_typeid.find(type_name);
}
TypeId TypeMgr::GetTypeIdByName_or_unresolve(std::string type_name) {
	auto iter = m_typename_2_typeid.find(type_name);
	if (iter != m_typename_2_typeid.end()) {
		return iter->second;
	}
	TypeId new_tid = allocate_typeid();
	log_info("unresolved type name[%s] tid[%d]", type_name.c_str(), new_tid);
	m_typename_2_typeid[type_name] = new_tid;
	m_typeinfos.push_back(nullptr);
	return new_tid;
}
TypeId TypeMgr::add_type(TypeInfo* ti) {
	assert(!HasTypeIdByName(ti->GetName()));

	ti->SetTypeId(allocate_typeid());
	m_typename_2_typeid[ti->GetName()] = ti->GetTypeId();
	log_info("add new type: name[%s] typeid[%d]", ti->GetName().c_str(), ti->GetTypeId());
	m_typeinfos.push_back(ti);

	return ti->GetTypeId();
}
TypeId TypeMgr::GetOrAddTypeFn(std::vector<Parameter> params, TypeId return_tid) {
	TypeInfoFn ti(params, return_tid);
	if (HasTypeIdByName(ti.GetName())) {
		return GetTypeIdByName(ti.GetName());
	}
	return add_type(new TypeInfoFn(params, return_tid));
}
