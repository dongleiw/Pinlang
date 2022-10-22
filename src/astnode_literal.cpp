#include "astnode_literal.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"

AstNodeLiteral::AstNodeLiteral(int32_t value) {
	m_value_int		= value;
	m_result_typeid = TYPE_ID_INT32;
}
AstNodeLiteral::AstNodeLiteral(int64_t value) {
	m_value_int		= value;
	m_result_typeid = TYPE_ID_INT64;
}
AstNodeLiteral::AstNodeLiteral(float value) {
	m_value_float	= value;
	m_result_typeid = TYPE_ID_FLOAT;
}
AstNodeLiteral::AstNodeLiteral(bool value) {
	m_value_bool	= value;
	m_result_typeid = TYPE_ID_BOOL;
}
AstNodeLiteral::AstNodeLiteral(std::string value) {
	m_value_str		= value;
	m_result_typeid = TYPE_ID_STR;
}
VerifyContextResult AstNodeLiteral::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	switch (m_result_typeid) {
	case TYPE_ID_INT32:
	{
		if (TYPE_ID_INT32 == vparam.GetResultTid() || TYPE_ID_INFER == vparam.GetResultTid()) {
			return VerifyContextResult(m_result_typeid, new Variable(int32_t(m_value_int)));
		} else if (TYPE_ID_INT64 == vparam.GetResultTid()) {
			m_result_typeid = TYPE_ID_INT64;
			return VerifyContextResult(m_result_typeid, new Variable(int64_t(m_value_int)));
		} else {
			panicf("wrong type");
		}
		break;
	}
	case TYPE_ID_INT64:
		return VerifyContextResult(m_result_typeid, new Variable(int64_t(m_value_int)));
		break;
	case TYPE_ID_FLOAT:
		return VerifyContextResult(m_result_typeid, new Variable(m_value_float));
		break;
	case TYPE_ID_BOOL:
		return VerifyContextResult(m_result_typeid, new Variable(m_value_bool));
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
	case TYPE_ID_INT32:
		return new Variable(int32_t(this->m_value_int));
		break;
	case TYPE_ID_INT64:
		return new Variable(int64_t(this->m_value_int));
		break;
	case TYPE_ID_FLOAT:
		return new Variable(this->m_value_float);
		break;
	case TYPE_ID_BOOL:
		return new Variable(this->m_value_bool);
		break;
	case TYPE_ID_STR:
		return new Variable(this->m_value_str);
		break;
	default:
		panicf("unknown literal type[%d]", m_result_typeid);
		break;
	}
}
AstNodeLiteral* AstNodeLiteral::DeepCloneT() {
	AstNodeLiteral* newone = new AstNodeLiteral();

	*newone = *this;

	return newone;
}
void AstNodeLiteral::CastToInt64() {
	assert(m_result_typeid == TYPE_ID_INT32);
	m_result_typeid = TYPE_ID_INT64;
}
