#include <assert.h>

#include "define.h"
#include "log.h"
#include "type.h"
#include "type_float.h"
#include "type_fn.h"
#include "type_int.h"
#include "type_mgr.h"
#include "type_restriction.h"
#include "type_str.h"
#include "type_type.h"
#include "verify_context.h"

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
	}
	{
		TypeInfoInt* ti_int = new TypeInfoInt();
		ti_int->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_int);
		assert(TYPE_ID_INT == ti_int->GetTypeId());
	}
	{
		TypeInfoFloat* ti_float = new TypeInfoFloat();
		ti_float->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_float);
		assert(TYPE_ID_FLOAT == ti_float->GetTypeId());
	}
	{
		TypeInfoStr* ti_str = new TypeInfoStr();
		ti_str->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_str);
		assert(TYPE_ID_STR == ti_str->GetTypeId());
	}

	// 定义内置restriction
	{
		//AddGenericRestriction(TypeInfoGenericRestriction::create_restriction_add());
		//AddGenericRestriction(TypeInfoGenericRestriction::create_restriction_sub());
		//AddGenericRestriction(TypeInfoGenericRestriction::create_restriction_mul());
		//AddGenericRestriction(TypeInfoGenericRestriction::create_restriction_div());
		//AddGenericRestriction(TypeInfoGenericRestriction::create_restriction_mod());
	}
}
void TypeMgr::InitBuiltinMethods(VerifyContext& ctx) {
	m_typeinfos.at(TYPE_ID_TYPE)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_INT)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_FLOAT)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_STR)->InitBuiltinMethods(ctx);
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
std::string TypeMgr::GetTypeName(std::vector<TypeId> vec_tid) const {
	std::string s;
	for (size_t i = 0; i < vec_tid.size(); i++) {
		s += GetTypeName(vec_tid.at(i));
		if (i < vec_tid.size() - 1) {
			s += ",";
		}
	}
	return s;
}
TypeId TypeMgr::add_type(TypeInfo* ti) {
	ti->SetTypeId(allocate_typeid());
	log_info("add new type: name[%s] typeid[%d]", ti->GetName().c_str(), ti->GetTypeId());
	m_typeinfos.push_back(ti);

	return ti->GetTypeId();
}
TypeId TypeMgr::GetOrAddTypeRestriction(TypeInfoRestriction* ti) {
	return add_type(ti);
}
TypeId TypeMgr::AddGenericType(TypeInfo* ti) {
	ti->SetTypeGroupId(TYPE_GROUP_ID_GENERIC_TYPE);
	return add_type(ti);
}
TypeId TypeMgr::AddTypeInfo(TypeInfo* ti) {
	return add_type(ti);
}
TypeId TypeMgr::GetOrAddTypeFn(std::vector<TypeId> params, TypeId return_tid) {
	return add_type(new TypeInfoFn(params, return_tid));
}
