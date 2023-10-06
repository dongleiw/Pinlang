#include "astnode_compile_const.h"
#include "astnode.h"
#include "define.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"

VerifyContextResult AstNodeCompileConst::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeCompileConst::Execute(ExecuteContext& ctx) {
	return m_const_value;
}
AstNodeCompileConst* AstNodeCompileConst::DeepCloneT() {
	return new AstNodeCompileConst(m_const_value);
}
