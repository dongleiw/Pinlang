#include "type_class.h"

#include <assert.h>
#include <map>
#include <vector>

#include "astnode_constraint.h"
#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

TypeInfoClass::TypeInfoClass(std::string class_name) {
	m_name		   = class_name;
	m_typegroup_id = TYPE_GROUP_ID_CLASS;
}
void TypeInfoClass::InitBuiltinMethods(VerifyContext& ctx) {
}
