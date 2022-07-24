#include "variable.h"
#include "type.h"

Variable::Variable(int value) {
	m_tid = TYPE_ID_INT;
	m_value_int = value;
}
