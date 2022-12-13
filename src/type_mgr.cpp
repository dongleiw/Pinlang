#include <assert.h>
#include <sys/types.h>

#include "define.h"
#include "log.h"
#include "type.h"
#include "type_array.h"
#include "type_bool.h"
#include "type_constraint.h"
#include "type_float.h"
#include "type_fn.h"
#include "type_int.h"
#include "type_mgr.h"
#include "type_str.h"
#include "type_tuple.h"
#include "type_type.h"
#include "verify_context.h"

TypeMgr g_typemgr;

TypeMgr::TypeMgr() {
}

void TypeMgr::InitTypes() {
	TypeId tid;
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
		tid = add_type(new TypeInfoInt(TYPE_ID_INT8));
		assert(TYPE_ID_INT8 == tid);

		tid = add_type(new TypeInfoInt(TYPE_ID_INT16));
		assert(TYPE_ID_INT16 == tid);

		tid = add_type(new TypeInfoInt(TYPE_ID_INT32));
		assert(TYPE_ID_INT32 == tid);

		tid = add_type(new TypeInfoInt(TYPE_ID_INT64));
		assert(TYPE_ID_INT64 == tid);

		tid = add_type(new TypeInfoInt(TYPE_ID_UINT8));
		assert(TYPE_ID_UINT8 == tid);

		tid = add_type(new TypeInfoInt(TYPE_ID_UINT16));
		assert(TYPE_ID_UINT16 == tid);

		tid = add_type(new TypeInfoInt(TYPE_ID_UINT32));
		assert(TYPE_ID_UINT32 == tid);

		tid = add_type(new TypeInfoInt(TYPE_ID_UINT64));
		assert(TYPE_ID_UINT64 == tid);
	}
	{
		TypeInfoFloat* ti_float = new TypeInfoFloat();
		ti_float->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_float);
		assert(TYPE_ID_FLOAT == ti_float->GetTypeId());
	}
	{
		TypeInfoBool* ti_bool = new TypeInfoBool();
		ti_bool->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_bool);
		assert(TYPE_ID_BOOL == ti_bool->GetTypeId());
	}
	{
		TypeInfoStr* ti_str = new TypeInfoStr();
		ti_str->SetTypeId(allocate_typeid());
		m_typeinfos.push_back(ti_str);
		assert(TYPE_ID_STR == ti_str->GetTypeId());
	}
	{
		TypeInfo* ti = new TypeInfo();
		ti->SetTypeId(allocate_typeid());
		ti->SetName("constraint");
		m_typeinfos.push_back(ti);
		assert(TYPE_ID_GENERIC_CONSTRAINT == ti->GetTypeId());
	}
	{
		TypeInfo* ti = new TypeInfo();
		ti->SetTypeId(allocate_typeid());
		ti->SetName("complexfn");
		m_typeinfos.push_back(ti);
		assert(TYPE_ID_COMPLEX_FN == ti->GetTypeId());
	}
}
void TypeMgr::InitBuiltinMethods(VerifyContext& ctx) {
	m_typeinfos.at(TYPE_ID_TYPE)->InitBuiltinMethods(ctx);

	m_typeinfos.at(TYPE_ID_INT8)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_INT16)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_INT32)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_INT64)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_UINT8)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_UINT16)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_UINT32)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_UINT64)->InitBuiltinMethods(ctx);

	m_typeinfos.at(TYPE_ID_FLOAT)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_BOOL)->InitBuiltinMethods(ctx);
	m_typeinfos.at(TYPE_ID_STR)->InitBuiltinMethods(ctx);

	bool   added		 = false;
	TypeId array_str_tid = GetOrAddTypeArray(ctx, TYPE_ID_STR, 0, added);
	//m_main_fn_tid		 = GetOrAddTypeFn(ctx, std::vector<TypeId>{array_str_tid}, TYPE_ID_INT32);
	m_main_fn_tid		 = GetOrAddTypeFn(ctx, std::vector<TypeId>{}, TYPE_ID_INT32);
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
TypeId TypeMgr::GetOrAddTypeConstraint(TypeInfoConstraint* ti) {
	return add_type(ti);
}
TypeId TypeMgr::AddGenericType(TypeInfo* ti) {
	ti->SetTypeGroupId(TYPE_GROUP_ID_VIRTUAL_GTYPE);
	return add_type(ti);
}
TypeId TypeMgr::AddTypeInfo(TypeInfo* ti) {
	return add_type(ti);
}
TypeId TypeMgr::GetOrAddTypeFn(VerifyContext& ctx, std::vector<TypeId> params, TypeId return_tid) {
	for (const auto ti : m_typeinfos) {
		if (ti->IsFn()) {
			const TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(ti);
			if (tifn->IsArgsTypeEqual(params) && tifn->GetReturnTypeId() == return_tid) {
				return tifn->GetTypeId();
			}
		}
	}
	TypeInfoFn* ti = new TypeInfoFn(params, return_tid);
	add_type(ti);
	ti->InitBuiltinMethods(ctx);
	return ti->GetTypeId();
}
TypeId TypeMgr::GetOrAddTypeArray(VerifyContext& ctx, TypeId element_tid, uint64_t static_size, bool& added) {
	added = false;
	for (const auto ti : m_typeinfos) {
		if (ti->IsArray()) {
			const TypeInfoArray* tiarray = dynamic_cast<TypeInfoArray*>(ti);
			if (tiarray->GetElementType() == element_tid && tiarray->GetStaticSize() == static_size) {
				return ti->GetTypeId();
			}
		}
	}
	added			  = true;
	TypeInfoArray* ti = new TypeInfoArray(element_tid, static_size);
	add_type(ti);
	ti->InitBuiltinMethods(ctx);
	return ti->GetTypeId();
}
bool TypeMgr::IsTypeExist(TypeId tid) const {
	return 0 < tid && tid < m_typeinfos.size();
}
TypeId TypeMgr::GetOrAddTypeTuple(VerifyContext& ctx, std::vector<TypeId> element_tids, bool& added) {
	added = false;
	for (const auto ti : m_typeinfos) {
		if (ti->IsTuple()) {
			const TypeInfoTuple* ti_tuple = dynamic_cast<TypeInfoTuple*>(ti);
			if (is_vec_typeid_equal(ti_tuple->GetElementTids(), element_tids)) {
				return ti->GetTypeId();
			}
		}
	}
	added			  = true;
	TypeInfoTuple* ti = new TypeInfoTuple(element_tids);
	add_type(ti);
	ti->InitBuiltinMethods(ctx);
	return ti->GetTypeId();
}
