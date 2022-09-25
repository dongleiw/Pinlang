#pragma once

#include <map>
#include <string>

#include "attr.h"
#include "define.h"

class Function;
class VerifyContext;

/*
 * 类型信息
 */
class TypeInfo {
public:
	struct Method {
		std::string method_name; // 方法名
		Function*	fn;			 // 方法的实现
	};
	struct Constraint {
		TypeId				constraint_tid;
		std::vector<Method> methods;
	};

public:
	TypeInfo() : m_typeid(TYPE_ID_NONE), m_typegroup_id(TYPE_GROUP_ID_UNRESOLVE) {
	}

	TypeId		GetTypeId() const { return m_typeid; }
	void		SetTypeId(TypeId tid) { m_typeid = tid; }
	std::string GetName() { return m_name; }
	void		SetName(std::string name) { m_name = name; }

	/*
	 * 根据方法名和参数类型查找函数, 如果出现多个匹配的情况则panic
	 */
	MethodIndex GetMethodIdx(std::string method_name, std::vector<TypeId> args_tid) const;
	/*
	 * 根据方法名和参数类型查找函数, 如果出现多个匹配的情况则panic
	 */
	MethodIndex GetMethodIdx(std::string method_name, TypeId tid) const;
	/*
	 * 根据参数类型查找属于某个constraint的某个方法
	 */
	MethodIndex GetMethodIdx(TypeId constraint_tid, std::string method_name, std::vector<TypeId> args_tid) const;
	/*
	 * 根据类型查找属于某个constraint的某个方法
	 */
	MethodIndex GetMethodIdx(TypeId constraint_tid, std::string method_name, TypeId tid) const;
	/*
	 * 只根据函数名查找. 找到多个会panic
	 */
	MethodIndex GetMethodIdx(std::string method_name) const;

	Function* GetMethodByIdx(MethodIndex method_idx);

	void		 AddConstraint(TypeId constraint_tid, std::map<std::string, Function*> methods);
	virtual void InitBuiltinMethods(VerifyContext& ctx) {}
	/*
	 * 创建新类型, 将类型中的泛型id替换为实际类型id
	 * 如果本身不是泛型, 返回nullptr
	 * 如果对应关系不全, 则panic
	 */
	virtual TypeInfo* ToConcreteType(std::map<TypeId, TypeId> gtid_2_ctid) const { return nullptr; }

	void SetTypeGroupId(TypeGroupId tgid) { m_typegroup_id = tgid; }

	bool IsFn() const { return m_typegroup_id == TYPE_GROUP_ID_FUNCTION; }
	bool IsArray() const { return m_typegroup_id == TYPE_GROUP_ID_ARRAY; }
	bool IsType() const { return m_typeid == TYPE_ID_TYPE; }
	bool IsConstraint() const { return m_typegroup_id == TYPE_GROUP_ID_CONSTRAINT; }
	bool IsGenericType() const { return m_typegroup_id == TYPE_GROUP_ID_VIRTUAL_GTYPE; }
	bool IsPrimaryType() const { return m_typegroup_id == TYPE_GROUP_ID_PRIMARY; }

	bool MatchConstraint(TypeId tid) const;

	bool HasField(std::string attr_name) const;

protected:
	TypeId		m_typeid;
	TypeGroupId m_typegroup_id;
	std::string m_name;

	/*
	 * 该类型实现的约束列表. 
	 */
	std::vector<Constraint> m_constraints;
};
