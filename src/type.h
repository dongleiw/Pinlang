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
	TYPE_ID_INFER = 1,
	TYPE_ID_INT	  = 2,
	TYPE_ID_FLOAT = 3,
	TYPE_ID_BOOL  = 4,
	TYPE_ID_STR	  = 5,
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
