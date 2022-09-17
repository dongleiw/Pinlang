#include "type_restriction.h"
#include "function.h"
#include "type_generic_type.h"
#include "type_mgr.h"

void TypeInfoRestriction::FillVirtualType(TypeInfoGenericType& ti) const {
	std::map<std::string, Function*> methods;
	for (auto rule : m_rules) {
		Function* f			  = new Function(rule.fn_tid, nullptr);
		methods[rule.fn_name] = f;
	}

	ti.AddRestriction(GetTypeId(), methods);
}
