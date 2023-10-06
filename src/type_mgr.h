#pragma once

#include "define.h"
#include "type.h"
#include "type_class.h"
#include "type_constraint.h"
#include "type_fn.h"
#include "verify_context.h"

#include <map>
#include <vector>

class TypeInfo;

class TypeMgr {
public:
	TypeMgr();

	/*
	 * 如果不存在, panic
	 */
	TypeInfo* GetTypeInfo(TypeId tid) const;
	bool	  IsTypeExist(TypeId tid) const;

	std::string GetTypeName(std::vector<TypeId> vec_tid) const;
	std::string GetTypeName(TypeId tid) const;

	/*
	 * 根据参数类型和返回类型获取函数类型
	 * 如果不存在, 增加一个
	 */
	TypeId GetOrAddTypeFn(VerifyContext& ctx, std::vector<TypeId> params, TypeId return_tid);

	TypeId GetOrAddTypeArray(VerifyContext& ctx, TypeId element_tid, uint64_t static_size);
	TypeId GetOrAddTypeTuple(VerifyContext& ctx, std::vector<TypeId> element_tids);
	TypeId GetOrAddTypePointer(VerifyContext& ctx, TypeId pointee_tid);
	TypeId AddTypeClass(TypeInfoClass* ti);

	TypeId AddTypeInfo(TypeInfo* ti);
	TypeId AddGenericType(TypeInfo* ti);

	TypeId AddConstraint(std::string constraint_name);

	void InitTypes();
	void InitBuiltinMethods(VerifyContext& ctx);

	TypeId GetMainFnTid() const { return m_main_fn_tid; }

private:
	/*
	 * 增加类型
	 * 如果已经存在, 则panic
	 * @ti 必须分配在堆
	 */
	TypeId add_type(TypeInfo* heap_alloc_ti);
	TypeId allocate_typeid();

	bool is_type_name_exists(std::string name) const;

private:
	std::vector<TypeInfo*> m_typeinfos;
	TypeId				   m_main_fn_tid;
};

extern TypeMgr g_typemgr;

#define GET_TYPENAME_C(tid) (g_typemgr.GetTypeName(tid).c_str())
#define GET_TYPENAME(tid) (g_typemgr.GetTypeName(tid))
