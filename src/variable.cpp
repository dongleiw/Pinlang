#include "variable.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"

Variable::Variable(int value) {
	m_tid		= TYPE_ID_INT;
	m_value_int = value;
}
Variable::Variable(float value) {
	m_tid		= TYPE_ID_FLOAT;
	m_value_float = value;
}
Variable::Variable(std::string value) {
	m_tid		= TYPE_ID_STR;
	m_value_str = value;
}
Variable::Variable(Function* fn) {
	m_tid	   = fn->GetTypeId();
	m_value_fn = fn;
}
Variable* Variable::CreateTypeVariable(TypeId tid) {
	Variable* v	   = new Variable(TYPE_ID_TYPE);
	v->m_value_tid = tid;
	return v;
}
Variable* Variable::CallMethod(ExecuteContext& ctx, int method_idx, std::vector<Variable*> args) {
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_tid);
	Function* f	 = ti->GetMethodByIdx(method_idx);
	f->SetThisObj(this);
	return f->Call(ctx, args);
}
std::string Variable::ToString() const {
	std::string s;
	char		buf[128];
	switch (m_tid) {
	case TYPE_ID_TYPE:
		snprintf(buf, sizeof(buf) - 1, "type(%d:%s)", m_value_tid, GET_TYPENAME_C(m_value_tid));
		s += buf;
		break;
	case TYPE_ID_INT:
		snprintf(buf, sizeof(buf) - 1, "int(%d)", m_value_int);
		s += buf;
		break;
	case TYPE_ID_STR:
		snprintf(buf, sizeof(buf) - 1, "str(%s)", m_value_str.c_str());
		s += buf;
		break;
	case TYPE_ID_FLOAT:
		snprintf(buf, sizeof(buf) - 1, "float(%f)", m_value_float);
		s += buf;
		break;
	//case TYPE_ID_BOOL:
	//	break;
	default:
		snprintf(buf, sizeof(buf) - 1, "unknown");
		s += buf;
		break;
	}
	return s;
}
