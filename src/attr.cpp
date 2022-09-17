#include "attr.h"

void Attr::InitMethod(TypeId contraint_tid, std::string method_name, Function* fn) {
	is_field			  = false;
	method.constraint_tid = contraint_tid;
	method.method_name	  = method_name;
	method.fn			  = fn;
}
