#include <cstdlib>
#include <ios>
#include <iterator>

#include "astnode_literal.h"
#include "astnode_type.h"
#include "astnode_vardef.h"
#include "define.h"
#include "log.h"
#include "support/CPPUtils.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

AstNodeVarDef::AstNodeVarDef(std::string var_name, AstNodeType* declared_type, AstNode* init_expr, bool is_const) {
	m_varname		= var_name;
	m_declared_type = declared_type;
	m_init_expr		= init_expr;
	m_is_const		= is_const;
}
VerifyContextResult AstNodeVarDef::Verify(VerifyContext& ctx) {
	log_debug("verify vardef: varname[%s]", m_varname.c_str());

	// VerifyContextParam param = ctx.GetParam();

	TypeId declared_tid = TYPE_ID_INFER;
	if (m_declared_type != nullptr) {
		VerifyContextResult vr = m_declared_type->Verify(ctx);
		declared_tid = vr.GetResultTypeId();
	}

	if (m_init_expr != nullptr) {
		const VerifyContextResult vr_init_expr = m_init_expr->Verify(ctx.SetParam(VerifyContextParam(declared_tid)));
		if (vr_init_expr.GetResultTypeId() == TYPE_ID_NONE) {
			panicf("init expr should be expression");
		}
		if (declared_tid == TYPE_ID_INFER) {
		} else {
			if (declared_tid != vr_init_expr.GetResultTypeId()) {
				panicf("var[%s] declared type[%s] != init expr type[%s]", m_varname.c_str(), GET_TYPENAME_C(declared_tid),
					   GET_TYPENAME_C(vr_init_expr.GetResultTypeId()));
			}
		}
		m_result_typeid = vr_init_expr.GetResultTypeId();
	} else if (declared_tid != TYPE_ID_INFER) {
		m_result_typeid = declared_tid;
	} else {
		panicf("var[%s] need type", m_varname.c_str())
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
