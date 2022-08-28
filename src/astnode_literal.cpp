#include "astnode_literal.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

AstNodeLiteral::AstNodeLiteral(int value) {
	m_value_int		= value;
	m_result_typeid = TYPE_ID_INT;
}
AstNodeLiteral::AstNodeLiteral(float value) {
	m_value_float		= value;
	m_result_typeid = TYPE_ID_FLOAT;
}
AstNodeLiteral::AstNodeLiteral(std::string value) {
	m_value_str		= value;
	m_result_typeid = TYPE_ID_STR;
}
VerifyContextResult AstNodeLiteral::Verify(VerifyContext& ctx) {
	switch (m_result_typeid) {
	case TYPE_ID_INT:
		return VerifyContextResult(m_result_typeid, new Variable(m_value_int));
		break;
	case TYPE_ID_FLOAT:
		return VerifyContextResult(m_result_typeid, new Variable(m_value_float));
		break;
	case TYPE_ID_STR:
		return VerifyContextResult(m_result_typeid, new Variable(m_value_str));
		break;
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
}
Variable* AstNodeLiteral::Execute(ExecuteContext& ctx) {
	switch (m_result_typeid) {
	case TYPE_ID_INT:
		return new Variable(this->m_value_int);
		break;
	case TYPE_ID_FLOAT:
		return new Variable(this->m_value_float);
		break;
	case TYPE_ID_STR:
		return new Variable(this->m_value_str);
		break;
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
}
