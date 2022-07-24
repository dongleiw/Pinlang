#include <cstdlib>
#include <ios>

#include "astnode_vardef.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

AstNodeVarDef::AstNodeVarDef(std::string var_name, TypeId declared_tid, AstNode* init_expr) {
	m_varname	   = var_name;
	m_declared_tid = declared_tid;
	m_init_expr	   = init_expr;
}
VerifyContextResult AstNodeVarDef::Verify(VerifyContext& ctx) {
	log_debug("verify vardef: varname[%s]", m_varname.c_str());

	VerifyContextParam param = ctx.GetParam();

	if (m_init_expr != NULL) {
		VerifyContextResult vr_init_expr = m_init_expr->Verify(ctx.SetParam(VerifyContextParam(m_declared_tid)));
		if (m_declared_tid == TYPE_ID_INFER) {
			m_declared_tid = vr_init_expr.GetResultTypeId();
		} else {
			if (m_declared_tid != vr_init_expr.GetResultTypeId()) {
				panicf("var[%s] declared as type[%s] != init_expr type[%s]", m_varname.c_str(), g_typemgr.GetTypeDesc(m_declared_tid),
					   g_typemgr.GetTypeDesc(vr_init_expr.GetResultTypeId()));
			}
		}
	}

	return VerifyContextResult();
}
Variable* AstNodeVarDef::Execute(ExecuteContext& ctx) {
	Variable *v;
	if(m_init_expr!=NULL){
		v = m_init_expr->Execute(ctx);
	}else{
		v = new Variable(m_declared_tid);
	}
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_varname, v);
	return NULL;
}
