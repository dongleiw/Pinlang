#pragma once

#include <map>
#include <string>

#include "define.h"

class Function;

/*
 * 类型信息
 */
class TypeInfo {
public:
	TypeInfo() : m_typeid(TYPE_ID_NONE), m_typegroup_id(TYPE_GROUP_ID_UNRESOLVE) {
	}

	TypeId		GetTypeId() { return m_typeid; }
	void		SetTypeId(TypeId tid) { m_typeid = tid; }
	std::string GetName() { return m_name; }

	void		 AddMethod(std::string method_name, Function* function);
	bool		 HasMethod(std::string method_name) const;
	void		 AddBuiltinMethod(std::string method_name, std::vector<TypeId> params_tid, TypeId ret_tid, BuiltinFnCallback cb);
	Function*	 GetMethodOrNilByName(std::string method_name);
	Function*	 GetMethodByName(std::string method_name);
	virtual void InitBuiltinMethods() {}

	void SetTypeGroupId(TypeGroupId tgid) { m_typegroup_id = tgid; }
	bool IsFn() const { return m_typegroup_id == TYPE_GROUP_ID_FUNCTION; }
	bool IsType() const { return m_typeid == TYPE_ID_TYPE; }

protected:
	TypeId		m_typeid;
	TypeGroupId m_typegroup_id;
	std::string m_name;
	/*
	 * 该类型的方法
	 * 方法没有定义顺序
	 */
	std::map<std::string, Function*> m_methods;
};
