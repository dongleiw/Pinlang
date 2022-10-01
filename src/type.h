#pragma once

#include <map>
#include <string>

#include "define.h"

class Function;
class VerifyContext;
class AstNodeComplexFnDef;

/*
 * 类型信息
 */
class TypeInfo {
public:
	// 类型的方法
	struct Method {
		std::string			 method_name;
		AstNodeComplexFnDef* method_node;
	};
	// 类型的方法的一个具体实例
	struct MethodInstance{
		std::string method_name; // 方法名
		Function*	fn;			 // 方法的实现
	};
	struct Constraint {
		TypeId						constraint_tid;
		std::vector<Method>			methods;
		std::vector<MethodInstance> concrete_methods;
		MethodIndex AddConcreteMethod(std::string method_name, Function* fn);
	};
	struct Field {
		std::string name;
		TypeId		tid;
	};

public:
	TypeInfo() : m_typeid(TYPE_ID_NONE), m_typegroup_id(TYPE_GROUP_ID_UNRESOLVE) {
	}

	TypeId		GetTypeId() const { return m_typeid; }
	void		SetTypeId(TypeId tid) { m_typeid = tid; }
	std::string GetName() { return m_name; }
	void		SetName(std::string name) { m_name = name; }

	/*
	 * 根据方法名和参数类型查找函数. 如果需要则实例化
	 */
	MethodIndex GetConcreteMethod(VerifyContext& ctx, std::string method_name, std::vector<TypeId> args_tid, TypeId return_tid);
	/*
	 * 根据方法名和函数类型查找函数. 如果需要则实例化
	 */
	MethodIndex GetConcreteMethod(VerifyContext& ctx, std::string method_name, TypeId tid);
	/*
	 * 仅仅根据方法名查找函数. 如果需要则实例化
	 */
	MethodIndex GetConcreteMethod(VerifyContext& ctx, std::string method_name);
	/*
	 * 只根据函数名查找. 找到多个会panic
	 */
	MethodIndex GetMethodIdx(std::string method_name) const;

	Function* GetMethodByIdx(MethodIndex method_idx);

	void		 AddConstraint(TypeId constraint_tid, std::vector<AstNodeComplexFnDef*> methods);
	virtual void InitBuiltinMethods(VerifyContext& ctx) {}

	void SetTypeGroupId(TypeGroupId tgid) { m_typegroup_id = tgid; }

	bool IsFn() const { return m_typegroup_id == TYPE_GROUP_ID_FUNCTION; }
	bool IsArray() const { return m_typegroup_id == TYPE_GROUP_ID_ARRAY; }
	bool IsType() const { return m_typeid == TYPE_ID_TYPE; }
	bool IsConstraint() const { return m_typegroup_id == TYPE_GROUP_ID_CONSTRAINT; }
	bool IsGenericType() const { return m_typegroup_id == TYPE_GROUP_ID_VIRTUAL_GTYPE; }
	bool IsPrimaryType() const { return m_typegroup_id == TYPE_GROUP_ID_PRIMARY; }

	bool MatchConstraint(TypeId tid) const;

	bool			   HasField(std::string field_name) const;
	void			   AddField(std::string field_name, TypeId tid);
	TypeId			   GetFieldType(std::string field_name) const;
	std::vector<Field> GetField() const { return m_field_list; }

protected:
	TypeId		m_typeid;
	TypeGroupId m_typegroup_id;
	std::string m_name;

	/*
	 * 该类型实现的约束列表. 
	 */
	std::vector<Constraint> m_constraints;

	std::vector<Field> m_field_list;
};
