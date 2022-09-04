#include "astnode_restriction.h"
#include "define.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"

VerifyContextResult AstNodeRestriction::Verify(VerifyContext& ctx) {
	log_debug("verify restriction[%d]", m_restriction_tid);

	// test
	{
		//TypeInfo* ti			  = g_typemgr.GetTypeInfo(TYPE_ID_INT);
		//TypeId	  restriction_tid = g_typemgr.GetTypeIdByName("stringify");
		//int		  method_idx	  = ti->GetMethodIdx(restriction_tid, "ToString", std::vector<TypeId>());
		//if (method_idx < 0) {
		//} else {
		//	Function* f = ti->GetMethodByIdx(method_idx);
		//}
	}
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeRestriction::Execute(ExecuteContext& ctx) {
	return nullptr;
}
