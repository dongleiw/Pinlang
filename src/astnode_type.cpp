#include "astnode_type.h"
#include "define.h"
#include "log.h"
#include "type.h"
#include "type_array.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_tuple.h"
#include "variable.h"
#include "verify_context.h"
#include <array>
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
void AstNodeType::InitWithArray(AstNodeType* element_type) {
	m_type_kind	   = TYPE_KIND_ARRAY;
	m_element_type = element_type;
}
void AstNodeType::InitWithTuple(std::vector<AstNodeType*> tuple_element_types) {
	m_type_kind			  = TYPE_KIND_TUPLE;
	m_tuple_element_types = tuple_element_types;
}
void AstNodeType::InitWithTargetTypeId(TypeId tid) {
	m_type_kind	 = TYPE_KIND_TARGET_TYPE_ID;
	m_target_tid = tid;
}

/*
 * 调用函数
 */
VerifyContextResult AstNodeType::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
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
			fn_params_tid.push_back(iter.type->Verify(ctx, VerifyContextParam()).GetResultTypeId());
		}
		TypeId fn_return_tid = TYPE_ID_NONE;
		if (m_fn_return_type != nullptr) {
			fn_return_tid = m_fn_return_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		}

		TypeId fn_tid = g_typemgr.GetOrAddTypeFn(ctx, fn_params_tid, fn_return_tid);
		vr.SetResultTypeId(fn_tid);
		break;
	}
	case TYPE_KIND_ARRAY:
	{
		TypeId element_tid = m_element_type->Verify(ctx, VerifyContextParam()).GetResultTypeId();
		bool   added	   = false;
		TypeId array_tid   = g_typemgr.GetOrAddTypeArray(ctx, element_tid, added);
		vr.SetResultTypeId(array_tid);
		break;
	}
	case TYPE_KIND_TUPLE:
	{
		std::vector<TypeId> tuple_element_tids;
		for (auto iter : m_tuple_element_types) {
			tuple_element_tids.push_back(iter->Verify(ctx, VerifyContextParam()).GetResultTypeId());
		}
		bool   added	 = false;
		TypeId tuple_tid = g_typemgr.GetOrAddTypeTuple(ctx, tuple_element_tids, added);
		vr.SetResultTypeId(tuple_tid);
		break;
	}
	case TYPE_KIND_TARGET_TYPE_ID:
	{
		vr.SetResultTypeId(m_target_tid);
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
		break;
	case TYPE_KIND_IDENTIFIER:
		if (target_tid == TYPE_ID_NONE) {
			panicf("target type id is none");
		}
		result[m_id] = target_tid;
		break;
	case TYPE_KIND_FN:
	{
		// 已知fn(a1 T1, a2 T2, ..., an TN)TN+1 的类型id为target_tid. 推导T1,T2,...,TN,TN+1
		TypeInfoFn* ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(target_tid));
		if (ti->GetParamNum() != m_fn_params.size()) {
			panicf("param number not equal");
		}
		for (size_t i = 0; i < ti->GetParamNum(); i++) {
			merge_infer_result(result, m_fn_params.at(i).type->InferType(ti->GetParamType(i)));
		}
		if (ti->GetReturnTypeId() == TYPE_ID_NONE && m_fn_return_type == nullptr) {
		} else if (ti->GetReturnTypeId() != TYPE_ID_NONE && m_fn_return_type != nullptr) {
			merge_infer_result(result, m_fn_return_type->InferType(ti->GetReturnTypeId()));
		} else {
			panicf("bug unknown state");
		}
		break;
	}
	case TYPE_KIND_ARRAY:
	{
		// 已知[]T类型id为target_tid. 推导T
		TypeInfoArray* ti				  = dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(target_tid));
		TypeId		   element_target_tid = ti->GetElementType();
		merge_infer_result(result, m_element_type->InferType(element_target_tid));
		break;
	}
	case TYPE_KIND_TUPLE:
	{
		// 已知tuple(T1,T2,...)类型id为target_tid. 推导T1,T2,...
		TypeInfoTuple*		ti				   = dynamic_cast<TypeInfoTuple*>(g_typemgr.GetTypeInfo(target_tid));
		std::vector<TypeId> tuple_element_tids = ti->GetElementTids();
		if (tuple_element_tids.size() != m_tuple_element_types.size()) {
			panicf("element number of tuple is not equal. failed to infer");
		}
		for (size_t i = 0; i < tuple_element_tids.size(); i++) {
			TypeId		 tuple_element_tid	= tuple_element_tids.at(i);
			AstNodeType* tuple_element_type = m_tuple_element_types.at(i);
			merge_infer_result(result, tuple_element_type->InferType(tuple_element_tid));
		}
		break;
	}
	default:
		panicf("unknown type_kind[%d]", m_type_kind);
		break;
	}
	return result;
}
AstNodeType* AstNodeType::DeepCloneT() {
	AstNodeType* newone = new AstNodeType();

	newone->m_type_kind = m_type_kind;
	switch (m_type_kind) {
	case TYPE_KIND_TYPE:
		newone->m_id = m_id;
		break;
	case TYPE_KIND_IDENTIFIER:
		newone->m_id = m_id;
		break;
	case TYPE_KIND_FN:
		for (auto iter : m_fn_params) {
			newone->m_fn_params.push_back(iter.DeepClone());
		}
		if (m_fn_return_type) {
			newone->m_fn_return_type = m_fn_return_type->DeepCloneT();
		}
		break;
	case TYPE_KIND_ARRAY:
		newone->m_element_type = m_element_type->DeepCloneT();
		break;
	case TYPE_KIND_TUPLE:
		for (auto iter : m_tuple_element_types) {
			newone->m_tuple_element_types.push_back(iter->DeepCloneT());
		}
		break;
	case TYPE_KIND_TARGET_TYPE_ID:
		newone->m_target_tid = m_target_tid;
		break;
	default:
		panicf("unknown type_kind[%d]", m_type_kind);
		break;
	}

	return newone;
}
void AstNodeType::merge_infer_result(std::map<std::string, TypeId>& to, std::map<std::string, TypeId> another) const {
	for (auto iter : another) {
		auto found = to.find(iter.first);
		if (found == to.end()) {
			to[iter.first] = iter.second;
		} else if (found->second == iter.second) {
		} else {
			panicf("infer type encounter conflict: typename[%s] => {%d:%s,%d:%s}", iter.first.c_str(), iter.second, GET_TYPENAME_C(iter.second), found->second, GET_TYPENAME_C(found->second));
		}
	}
}
