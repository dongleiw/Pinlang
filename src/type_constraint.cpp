#include "type_constraint.h"
#include "function.h"
#include "type_mgr.h"
#include "type_virtual_gtype.h"

void TypeInfoConstraint::FillVirtualType(TypeInfoVirtualGType& ti) const {
	std::map<std::string, Function*> methods;
	for (auto rule : m_rules) {
		Function* f			  = new Function(rule.fn_tid, std::vector<ConcreteGParam>(), nullptr);
		methods[rule.fn_name] = f;
	}

	ti.AddConstraint(GetTypeId(), methods);
}
