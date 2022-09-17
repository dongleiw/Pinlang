#include "type_restriction.h"
#include "function.h"
#include "type_virtual_gtype.h"
#include "type_mgr.h"

void TypeInfoRestriction::FillVirtualType(TypeInfoVirtualGType& ti) const {
	std::map<std::string, Function*> methods;
	for (auto rule : m_rules) {
		Function* f			  = new Function(rule.fn_tid, nullptr);
		methods[rule.fn_name] = f;
	}

	ti.AddRestriction(GetTypeId(), methods);
}
