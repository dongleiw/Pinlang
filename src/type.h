#pragma once

#include <map>
#include <string>

class Function;
/*
 * 类型的分类(tgid)
 */
enum TypeGroupId {
	/*
	 * 未完成定义的类别. 由于解析的顺序问题, 可能类型的使用先于类型的定义. 这种会先临时标记为unresolved
	 * 在后续解析到类型定义时, 在修正为实际类型
	 */
	TYPE_GROUP_ID_UNRESOLVE = 0,
	TYPE_GROUP_ID_PRIMARY	= 1,
	TYPE_GROUP_ID_ARRAY		= 2,
	TYPE_GROUP_ID_FUNCTION	= 3,
	TYPE_GROUP_ID_CLASS		= 4,
};

/*
 * 类型id (tid)
 * 每个类型都有一个唯一id
 */
enum TypeId {
	TYPE_ID_NONE  = 0,
	TYPE_ID_INFER = 1, // 推导类型. 在Verify阶段, 所有这种类型的变量都将变更为实际具体类型
	TYPE_ID_TYPE  = 2, // type类型.
	TYPE_ID_INT	  = 3,
	TYPE_ID_FLOAT = 4,
	TYPE_ID_BOOL  = 5,
	TYPE_ID_STR	  = 6,
};

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
