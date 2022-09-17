#include "astnode_generic_instantiate.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_mgr.h"
#include "variable.h"
#include "astnode_generic_fndef.h"
#include "function.h"
#include <cassert>

AstNodeGenericInstantiate::AstNodeGenericInstantiate(std::string generic_name, std::vector<AstNodeType*> type_list) {
	m_generic_name = generic_name;
	m_type_list	   = type_list;
}
VerifyContextResult AstNodeGenericInstantiate::Verify(VerifyContext& ctx) {
	log_debug("begin to verify generic instantiate");

	// 得到泛参的实际类型
	std::vector<TypeId> gparams_tid;
	for(auto iter:m_type_list){
		gparams_tid.push_back( iter->Verify(ctx).GetResultTypeId());
	}

	Variable* v	 = ctx.GetCurStack()->GetVariable(m_generic_name);
	TypeInfo* ti = g_typemgr.GetTypeInfo(v->GetTypeId());
	if (ti->GetTypeId() == TYPE_ID_GENERIC_FN) {
		// 泛型函数
		AstNodeGenericFnDef* astnode_generic_fndef = v->GetValueGenericFnDef();
		AstNodeGenericFnDef::Instance instance = astnode_generic_fndef->Instantiate(ctx, gparams_tid);
		m_result_typeid = instance.fnobj.GetFunction()->GetTypeId();
		m_instance_name = instance.instance_name;
	} else {
		panicf("bug");
	}
	VerifyContextResult vr(m_result_typeid);
	return vr;
}
Variable* AstNodeGenericInstantiate::Execute(ExecuteContext& ctx) {
	return ctx.GetCurStack()->GetVariable(m_instance_name);
}
AstNodeGenericInstantiate* AstNodeGenericInstantiate::DeepCloneT(){
	AstNodeGenericInstantiate * newone = new AstNodeGenericInstantiate();

	newone->m_generic_name = m_generic_name;
	for(auto iter:m_type_list){
		newone->m_type_list.push_back(iter->DeepCloneT());
	}

	return newone;
}