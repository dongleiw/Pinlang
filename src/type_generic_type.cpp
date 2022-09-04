#include "type_generic_type.h"

#include <assert.h>
#include <map>
#include <vector>

#include "define.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"

TypeInfoGenericType::TypeInfoGenericType(std::string name) {
	m_name		   = name;
	m_typegroup_id = TYPE_GROUP_ID_GENERIC_TYPE;
}
TypeInfo* TypeInfoGenericType::ToConcreteType(std::map<TypeId, TypeId> gtid_2_ctid) const {
	auto found = gtid_2_ctid.find(m_typeid);
	if(found==gtid_2_ctid.end()){
		panicf("no concrete typeid to replace generic type[%d:%s]", m_typeid, m_name.c_str());
	}
	return g_typemgr.GetTypeInfo(found->second);
}
