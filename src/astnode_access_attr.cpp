#include "astnode_access_attr.h"
#include "define.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"

AstNodeAccessAttr::AstNodeAccessAttr(AstNode* obj_expr, std::string attr_name) {
	m_obj_expr	= obj_expr;
	m_attr_name = attr_name;
	m_obj_expr->SetParent(this);
}
VerifyContextResult AstNodeAccessAttr::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify access attr[%s]", m_attr_name.c_str());

	VerifyContextResult vr_expr = m_obj_expr->Verify(ctx, VerifyContextParam());
	TypeId				obj_tid = vr_expr.GetResultTypeId();
	TypeInfo*			ti		= g_typemgr.GetTypeInfo(obj_tid);
	if (ti->HasField(m_attr_name)) {
		// 是字段
		m_is_field		= true;
		m_result_typeid = ti->GetFieldType(m_attr_name);
	} else {
		m_is_field = false;
		// 是方法
		if (vparam.HasFnCallArgs()) {
			// 父节点传递过来了函数调用的参数类型
			// 这说明该变量是函数
			// 根据参数类型和结果类型来选择
			MethodIndex method_idx = ti->GetConcreteMethod(ctx, m_attr_name, vparam.GetFnCallArgs(), vparam.GetResultTid());
			if (!method_idx.IsValid()) {
				panicf("type[%d:%s] doesn't have method[%s] with args[%s]", obj_tid, GET_TYPENAME_C(obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			m_method_idx	= method_idx;
			m_result_typeid = ti->GetMethodByIdx(method_idx)->GetTypeId();
		} else if (vparam.GetResultTid() != TYPE_ID_INFER) {
			// 父节点传递过来了期望的结果类型
			// 使用该类型来选择合适的函数重载
			MethodIndex method_idx = ti->GetConcreteMethod(ctx, m_attr_name, vparam.GetResultTid());
			if (!method_idx.IsValid()) {
				panicf("type[%d:%s] doesn't have method[%s] of type[%d:%s]", obj_tid, GET_TYPENAME_C(obj_tid), m_attr_name.c_str(), vparam.GetResultTid(), GET_TYPENAME_C(vparam.GetResultTid()));
			}
			m_method_idx	= method_idx;
			m_result_typeid = ti->GetMethodByIdx(method_idx)->GetTypeId();
		} else {
			// 上下文不足无法推断. 最后尝试下只用方法名查找, 如果有多个重名方法, 则失败
			MethodIndex method_idx = ti->GetConcreteMethod(ctx, m_attr_name);
			if (!method_idx.IsValid()) {
				panicf("type[%d:%s] doesn't have method[%s]", obj_tid, GET_TYPENAME_C(obj_tid), m_attr_name.c_str());
			}
			m_method_idx	= method_idx;
			m_result_typeid = ti->GetMethodByIdx(method_idx)->GetTypeId();
		}
	}
	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeAccessAttr::Execute(ExecuteContext& ctx) {
	Variable* v = m_obj_expr->Execute(ctx);
	if (m_is_field) {
		return v->GetFieldValue(m_attr_name);
	} else {
		return v->GetMethodValue(m_method_idx);
	}
}
AstNodeAccessAttr* AstNodeAccessAttr::DeepCloneT() {
	AstNodeAccessAttr* newone = new AstNodeAccessAttr(m_obj_expr->DeepClone(), m_attr_name);
	return newone;
}
