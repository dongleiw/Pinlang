#pragma once

#include <string>

enum TypeGroupId {
	TYPE_GROUP_ID_PRIMARY  = 1,
	TYPE_GROUP_ID_ARRAY	   = 2,
	TYPE_GROUP_ID_FUNCTION = 3,
	TYPE_GROUP_ID_CLASS	   = 4,
};

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
	TypeInfo() {}

	TypeId		GetTypeId() { return m_typeid; }
	void		SetTypeId(TypeId tid) { m_typeid = tid; }
	std::string GetDesc() { return m_desc; }

  protected:
	TypeId		m_typeid;
	std::string m_desc;
};
