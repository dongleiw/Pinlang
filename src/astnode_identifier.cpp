#include "astnode_identifier.h"
#include "variable.h"

#include "log.h"

VerifyContextResult AstNodeIdentifier::Verify(VerifyContext& ctx) {
	log_debug("verify identifier[%s]", m_id.c_str());

	Variable* v = ctx.GetCurStack()->GetVariableOrNull(m_id);
	if (v == NULL) {
		panicf("var[%s] not exist", m_id.c_str());
	} else {
		m_result_typeid = v->GetTypeId();
	}

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeIdentifier::Execute(ExecuteContext& ctx) {
	return  ctx.GetCurStack()->GetVariable(m_id);
}
