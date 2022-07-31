#include "astnode_fndef.h"
#include "astnode_blockstmt.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "variable_table.h"

AstNodeFnDef::AstNodeFnDef(TypeId tid, std::string fn_name, std::vector<std::string> params_name, AstNodeBlockStmt* body) {
	m_tid		  = tid;
	m_fnname	  = fn_name;
	m_params_name = params_name;
	m_body		  = body;
}
VerifyContextResult AstNodeFnDef::Verify(VerifyContext& ctx) {
	log_debug("begin to verify fndef: fnname[%s]", m_fnname.c_str());

	// 检查函数名是否冲突
	if (ctx.GetCurStack()->IsVariableExist(m_fnname)) {
		panicf("fnname[%s] conflict", m_fnname.c_str());
	}

	ctx.PushStack();
	{
		// 将参数放入vt
		VariableTable* vt_args = new VariableTable();
		TypeInfoFn*	   tifn	   = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_tid));
		for (size_t i = 0; i < tifn->GetParamNum(); i++) {
			Variable* v = new Variable(tifn->GetParamType(i));
			vt_args->AddVariable(m_params_name.at(i), v);
		}
		ctx.GetCurStack()->EnterBlock(vt_args);

		// 指定期望return的类型
		ctx.SetParam(VerifyContextParam(TYPE_ID_NONE, tifn->GetReturnTypeId()));
		m_body->Verify(ctx);
	}
	ctx.PopSTack();

	// 将函数放到vt
	Function* fn = new Function(m_tid, m_params_name, m_body);
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_fnname, new Variable(fn));

	log_debug("end to verify fndef: fnname[%s]", m_fnname.c_str());

	VerifyContextResult vr(m_result_typeid);
	return vr;
}
Variable* AstNodeFnDef::Execute(ExecuteContext& ctx) {
	Function* fn = new Function(m_tid, m_params_name, m_body);
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_fnname, new Variable(fn));
	return nullptr;
}
