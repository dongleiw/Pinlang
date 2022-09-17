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
	/*
	 * 类型的方法
	 */
	struct Method {
		TypeId		constraint_tid; // 方法所属constraint. 如果不属于constraint, 则为TYPE_ID_NONE
		std::string name;			// 方法名
		Function*	f;				// 方法的实现
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
	int GetMethodIdx(std::string method_name, std::vector<TypeId> args_tid) const;
	/*
	 * 根据方法名和参数类型查找函数, 如果出现多个匹配的情况则panic
	 */
	int GetMethodIdx(std::string method_name, TypeId tid) const;
	/*
	 * 根据参数类型查找属于某个constraint的某个方法
	 */
	int GetMethodIdx(TypeId constraint_tid, std::string method_name, std::vector<TypeId> args_tid) const;
	/*
	 * 根据类型查找属于某个constraint的某个方法
	 */
	int GetMethodIdx(TypeId constraint_tid, std::string method_name, TypeId tid) const;

	Function* GetMethodByIdx(int idx);

	void		 AddMethod(TypeId constraint_tid, std::string method_name, Function* function);
	void		 AddBuiltinMethod(TypeId constraint_tid, std::string method_name, std::vector<TypeId> params_tid, TypeId ret_tid, BuiltinFnCallback cb);
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
	bool IsType() const { return m_typeid == TYPE_ID_TYPE; }
	bool IsConstraint() const { return m_typegroup_id == TYPE_GROUP_ID_RESTRICTION; }
	bool IsGenericType() const { return m_typegroup_id == TYPE_GROUP_ID_GENERIC_TYPE; }

	bool MatchConstraint(TypeId tid) const;

	bool HasField(std::string attr_name) const;
	/*
	 * 如果只有一个名为attr_name的方法, attr_idx赋值为对应index. 
	 * 否则返回false, attr_idx不修改
	 */
	bool HasSingleMethod(std::string attr_name, size_t& attr_idx) const;
	Attr GetAttr(int attr_idx);

protected:
	bool has_duplicate_method(TypeId constraint_tid, std::string method_name, TypeId new_tid) const;

protected:
	TypeId		m_typeid;
	TypeGroupId m_typegroup_id;
	std::string m_name;

	/*
	 * 该类型的属性(包括字段和方法)
	 * 字段名不允许重复
	 * 方法可能会重复. 比如有重载, 或者多个约束有重名函数
	 */
	std::vector<Attr> m_attrs;
};
