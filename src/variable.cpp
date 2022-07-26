#include "variable.h"
#include "type.h"

Variable::Variable(int value) {
	m_tid		= TYPE_ID_INT;
	m_value_int = value;
}
Variable* Variable::CreateTypeVariable(TypeId tid) {
	Variable* v = new Variable(TYPE_ID_TYPE);
	v->m_value_tid = tid;
	return v;
}
