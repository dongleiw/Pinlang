#pragma once

#include <map>
#include <string>

#include "define.h"

class Function;
class VerifyContext;

/*
 * 类型信息
 */
class TypeInfo {
public:
	struct Restriction {
		TypeId							 tid;
		std::map<std::string, Function*> methods;
	};
	/*
	 * 类型的方法
	 */
	struct Method {
		TypeId		restriction_tid; // 方法所属restriction. 如果不属于restriction, 则为TYPE_ID_NONE
		std::string name;			 // 方法名
		Function*	f;				 // 方法的实现
	};

public:
	TypeInfo() : m_typeid(TYPE_ID_NONE), m_typegroup_id(TYPE_GROUP_ID_UNRESOLVE) {
	}

	TypeId		GetTypeId() { return m_typeid; }
	void		SetTypeId(TypeId tid) { m_typeid = tid; }
	std::string GetName() { return m_name; }
	void		SetName(std::string name) { m_name = name; }

	/*
	 * 根据方法名和参数类型查找函数, 如果出现多个匹配的情况则panic
	 */
	int GetMethodIdx(std::string method_name, std::vector<TypeId> args_tid) const;
	/*
	 * 根据参数类型查找属于某个restriction的某个方法
	 */
	int GetMethodIdx(TypeId restriction_tid, std::string method_name, std::vector<TypeId> args_tid) const;
	int GetMethodIdx(TypeId restriction_tid, std::string method_name, TypeId tid) const;

	Function* GetMethodByIdx(int idx);

	void		 AddMethod(TypeId restriction_tid, std::string method_name, Function* function);
	void		 AddBuiltinMethod(TypeId restriction_tid, std::string method_name, std::vector<TypeId> params_tid, TypeId ret_tid, BuiltinFnCallback cb);
	void		 AddRestriction(TypeId restriction_tid, std::map<std::string, Function*> methods);
	virtual void InitBuiltinMethods(VerifyContext& ctx) {}
	/*
	 * 创建新类型, 将类型中的泛型id替换为实际类型id
	 * 如果本身不是泛型, 返回nullptr
	 * 如果对应关系不全, 则panic
	 */
	virtual TypeInfo* ToConcreteType(std::map<TypeId, TypeId> gtid_2_ctid) const { return nullptr; }

	void SetTypeGroupId(TypeGroupId tgid) { m_typegroup_id = tgid; }

	bool IsFn() const { return m_typegroup_id == TYPE_GROUP_ID_FUNCTION; }
	bool IsType() const { return m_typeid == TYPE_ID_TYPE; }
	bool IsRestriction() const { return m_typegroup_id == TYPE_GROUP_ID_RESTRICTION; }
	bool IsGenericType() const { return m_typegroup_id == TYPE_GROUP_ID_GENERIC_TYPE; }

	bool MatchRestriction(TypeId tid) const;

protected:
	bool has_duplicate_method(TypeId restriction_tid, std::string method_name, TypeId new_tid) const;

protected:
	TypeId		m_typeid;
	TypeGroupId m_typegroup_id;
	std::string m_name;

	/*
	 * 该类型的方法
	 * 如果有重载, 会有多个同名方法
	 */
	std::vector<Method> m_methods;
};
