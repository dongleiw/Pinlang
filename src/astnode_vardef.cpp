#include <cstdlib>
#include <ios>
#include <iterator>

#include "astnode_literal.h"
#include "astnode_vardef.h"
#include "log.h"
#include "support/CPPUtils.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

AstNodeVarDef::AstNodeVarDef(std::string var_name, TypeId declared_tid, AstNode* init_expr, bool is_const) {
	m_varname	   = var_name;
	m_declared_tid = declared_tid;
	m_init_expr	   = init_expr;
	m_is_const	   = is_const;
}
VerifyContextResult AstNodeVarDef::Verify(VerifyContext& ctx) {
	log_debug("verify vardef: varname[%s]", m_varname.c_str());

	// VerifyContextParam param = ctx.GetParam();

	VerifyContextResult vr_init_expr;
	if (m_init_expr != nullptr) {
		vr_init_expr = m_init_expr->Verify(ctx.SetParam(VerifyContextParam(m_declared_tid)));
		if (m_declared_tid == TYPE_ID_INFER) {
		} else {
			if (m_declared_tid != vr_init_expr.GetResultTypeId()) {
				panicf("var[%s] declared type[%s] != init expr type[%s]", m_varname.c_str(), GET_TYPENAME_C(m_declared_tid),
					   GET_TYPENAME_C(vr_init_expr.GetResultTypeId()));
			}
		}
		m_result_typeid = vr_init_expr.GetResultTypeId();
	} else if (m_declared_tid != TYPE_ID_INFER) {
		m_result_typeid = m_declared_tid;
	} else {
		panicf("var[%s] need type", m_varname.c_str())
	}

	if (m_declared_tid == TYPE_ID_TYPE && m_is_const) {
		/*
		 * 定义了一个const的type类型的变量. 这种形式的变量可以作为类型使用, 需要注册
		 */
	}

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_varname, new Variable(m_result_typeid));
	return VerifyContextResult();
}
Variable* AstNodeVarDef::Execute(ExecuteContext& ctx) {
	Variable* v;
	if (m_init_expr != nullptr) {
		v = m_init_expr->Execute(ctx);
	} else {
		v = new Variable(m_result_typeid);
	}
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_varname, v);
	return nullptr;
}
