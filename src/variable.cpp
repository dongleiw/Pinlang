#include "variable.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"

Variable::Variable(int value) {
	m_tid		= TYPE_ID_INT;
	m_value_int = value;
}
Variable* Variable::CreateTypeVariable(TypeId tid) {
	Variable* v	   = new Variable(TYPE_ID_TYPE);
	v->m_value_tid = tid;
	return v;
}
Variable* Variable::CallMethod(ExecuteContext& ctx, std::string method_name, std::vector<Variable*> args) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	Function* f	 = ti->GetMethodByName(method_name);
	return f->Call(ctx, this, args);
}
std::string Variable::ToString() const {
	std::string s;
	char		buf[128];
	switch (m_tid) {
	case TYPE_ID_TYPE:
		snprintf(buf, sizeof(buf) - 1, "type(%d)", m_value_tid);
		s += buf;
		break;
	case TYPE_ID_INT:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", m_value_int);
		s += buf;
		break;
	case TYPE_ID_FLOAT:
		break;
	case TYPE_ID_STR:
		break;
	case TYPE_ID_BOOL:
		break;
	default:
		panicf("unexpected typeid[%d]", m_tid);
		break;
	}
	return s;
}
