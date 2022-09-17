#include "type_virtual_gtype.h"

#include <assert.h>
#include <map>
#include <vector>

#include "define.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"

TypeInfoVirtualGType::TypeInfoVirtualGType(std::string name) {
	m_name		   = name;
	m_typegroup_id = TYPE_GROUP_ID_GENERIC_TYPE;
}
