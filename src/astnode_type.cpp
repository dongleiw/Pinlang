#include "astnode_type.h"
#include "define.h"
#include "function.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"
#include <memory>
#include <vector>

void AstNodeType::InitWithType() {
	m_type_kind = TYPE_KIND_TYPE;
}
void AstNodeType::InitWithIdentifier(std::string id) {
	m_type_kind = TYPE_KIND_IDENTIFIER;
	m_id		= id;
}
void AstNodeType::InitWithFn(std::vector<ParserParameter> params, AstNodeType* return_type) {
	m_type_kind		 = TYPE_KIND_FN;
	m_fn_params		 = params;
	m_fn_return_type = return_type;
}

/*
 * 调用函数
 */
VerifyContextResult AstNodeType::Verify(VerifyContext& ctx) {
	log_debug("verify type: type_kind[%d] id[%s]", m_type_kind, m_id.c_str());
	VerifyContextResult vr;
	switch (m_type_kind) {
	case TYPE_KIND_TYPE:
		vr.SetResultTypeId(TYPE_ID_TYPE);
		break;
	case TYPE_KIND_IDENTIFIER:
	{
		//vr.SetResultTypeId(ctx.GetCurStack()->GetVariableType(m_id));
		Variable* v = ctx.GetCurStack()->GetVariable(m_id);
		vr.SetResultTypeId(v->GetValueTid());
		break;
	}
	case TYPE_KIND_FN:
	{
		std::vector<TypeId> fn_params_tid;
		for (auto iter : m_fn_params) {
			fn_params_tid.push_back(iter.type->Verify(ctx).GetResultTypeId());
		}
		TypeId fn_return_tid = TYPE_ID_NONE;
		if (m_fn_return_type != nullptr) {
			fn_return_tid = m_fn_return_type->Verify(ctx).GetResultTypeId();
		}

		TypeId fn_tid = g_typemgr.GetOrAddTypeFn(fn_params_tid, fn_return_tid);
		vr.SetResultTypeId(fn_tid);
		break;
	}
	default:
		panicf("unknown type_kind[%d]", m_type_kind);
		break;
	}
	return vr;
}
std::map<std::string, TypeId> AstNodeType::InferType(TypeId target_tid) const {
	std::map<std::string, TypeId> result;
	switch (m_type_kind) {
	case TYPE_KIND_TYPE:
		if (target_tid != TYPE_ID_TYPE) {
			panicf("bug");
		}
		return result;
		break;
	case TYPE_KIND_IDENTIFIER:
		result[m_id] = target_tid;
		return result;
		break;
	default:
		panicf("unknown type_kind[%d]", m_type_kind);
		break;
	}
}
AstNodeType* AstNodeType::DeepCloneT(){
	AstNodeType* newone = new AstNodeType();

	newone->m_type_kind = m_type_kind;
	newone->m_id = m_id;

	for(auto iter:m_fn_params){
		newone->m_fn_params.push_back(iter.DeepClone());
	}
	if(m_fn_return_type){
		newone->m_fn_return_type = m_fn_return_type->DeepCloneT();
	}

	return newone;
}
