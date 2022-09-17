#include "astnode_fndef.h"
#include "astnode.h"
#include "astnode_blockstmt.h"
#include "astnode_type.h"
#include "define.h"
#include "function.h"
#include "function_obj.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"

AstNodeFnDef::AstNodeFnDef(std::string fn_name, std::vector<ParserParameter> params, AstNodeType* return_type, AstNodeBlockStmt* body) {
	m_fnname	  = fn_name;
	m_params	  = params;
	m_return_type = return_type;
	m_body		  = body;

	m_body->SetParent(this);
}
VerifyContextResult AstNodeFnDef::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("begin to verify fndef: fnname[%s]", m_fnname.c_str());

	// 生成函数的typeid
	std::vector<TypeId> params;
	{
		for (auto iter : m_params) {
			TypeId param_tid = iter.type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
			params.push_back(param_tid);
		}
		TypeId return_tid = TYPE_ID_NONE;
		if (m_return_type != nullptr) {
			return_tid = m_return_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		}
		m_result_typeid = g_typemgr.GetOrAddTypeFn(params, return_tid);
	}

	// 检查函数名是否冲突
	if (ctx.GetCurStack()->IsVariableExist(m_fnname)) {
		panicf("fnname[%s] conflict", m_fnname.c_str());
	}
	TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(m_result_typeid));

	ctx.PushStack();
	{
		// 将参数放入vt
		VariableTable* vt_args = new VariableTable();
		for (size_t i = 0; i < tifn->GetParamNum(); i++) {
			Variable* v = new Variable(tifn->GetParamType(i));
			vt_args->AddVariable(m_params.at(i).name, v);
		}
		ctx.GetCurStack()->EnterBlock(vt_args);

		// 指定期望return的类型
		m_body->Verify(ctx, VerifyContextParam().SetReturnTid(tifn->GetReturnTypeId()));
	}
	ctx.PopSTack();

	// 将函数放到vt
	std::vector<std::string> params_name;
	for (auto iter : m_params) {
		params_name.push_back(iter.name);
	}
	Function*	fn	  = new Function(m_result_typeid, params_name, m_body);
	FunctionObj fnobj = FunctionObj(nullptr, fn);
	m_uniq_fnname	  = TypeInfoFn::GetUniqFnName(m_fnname, params);
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_uniq_fnname, new Variable(fnobj));
	ctx.GetCurStack()->GetCurVariableTable()->AddCandidateFn(m_fnname, fn);

	log_debug("end to verify fndef: fnname[%s] uniqfnname[%s]", m_fnname.c_str(), m_uniq_fnname.c_str());

	VerifyContextResult vr(m_result_typeid);
	return vr;
}
Variable* AstNodeFnDef::Execute(ExecuteContext& ctx) {
	std::vector<std::string> params_name;
	for (auto iter : m_params) {
		params_name.push_back(iter.name);
	}
	Function*	fn	  = new Function(m_result_typeid, params_name, m_body);
	FunctionObj fnobj = FunctionObj(nullptr, fn);
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_uniq_fnname, new Variable(fnobj));
	return nullptr;
}
AstNodeFnDef* AstNodeFnDef::DeepCloneT() {
	AstNodeFnDef* newone = new AstNodeFnDef();
	newone->m_fnname	 = m_fnname;
	for (auto iter : m_params) {
		newone->m_params.push_back(iter.DeepClone());
	}
	if (m_return_type != nullptr)
		newone->m_return_type = m_return_type->DeepCloneT();
	newone->m_body = m_body->DeepCloneT();
	return newone;
}
