#include "astnode_literal.h"
#include "type.h"
#include "variable.h"
#include "verify_context.h"


AstNodeLiteral::AstNodeLiteral(int value){
	m_value_int=value;
	m_result_typeid=TYPE_ID_INT;
}
VerifyContextResult AstNodeLiteral::Verify(VerifyContext& ctx){
	VerifyContextResult vr(m_result_typeid);

	return vr;
}
Variable* AstNodeLiteral::Execute(ExecuteContext& ctx){
	return new Variable(this->m_value_int);
}
