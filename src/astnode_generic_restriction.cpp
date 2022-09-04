#include "astnode_generic_restriction.h"
#include "define.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"

VerifyContextResult AstNodeGenericRestriction::Verify(VerifyContext& ctx) {
	log_debug("verify restriction");

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeGenericRestriction::Execute(ExecuteContext& ctx) {
	return nullptr;
}
