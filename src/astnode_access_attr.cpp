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
VerifyContextResult AstNodeAccessAttr::Verify(VerifyContext& ctx) {
	log_debug("verify access attr[%s]", m_attr_name.c_str());

	VerifyContextResult vr_expr = m_obj_expr->Verify(ctx);
	TypeId				obj_tid = vr_expr.GetResultTypeId();
	TypeInfo*			ti		= g_typemgr.GetTypeInfo(obj_tid);
	if (ti->HasField(m_attr_name)) {
		// 是字段
		panicf("not implemented");
		m_is_field = true;
	} else {
		m_is_field = false;
		// 是方法
		const VerifyContextParam vc_param = ctx.GetParam();
		if (vc_param.HasFnCallArgs()) {
			// 父节点传递过来了函数调用的参数类型
			// 这说明该变量是函数
			// 根据参数类型和结果类型来选择
			MethodIndex method_idx = ti->GetMethodIdx(m_attr_name, vc_param.GetFnCallArgs());
			if (!method_idx.IsValid()) {
				panicf("type[%d:%s] doesn't have method[%s] with args[%s]", obj_tid, GET_TYPENAME_C(obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vc_param.GetFnCallArgs()).c_str());
			}
			m_method_idx	= method_idx;
			m_result_typeid = ti->GetMethodByIdx(method_idx)->GetTypeId();
		} else if (vc_param.GetResultTid() != TYPE_ID_INFER) {
			// 父节点传递过来了期望的结果类型
			// 使用该类型来选择合适的函数重载
			MethodIndex method_idx = ti->GetMethodIdx(m_attr_name, vc_param.GetResultTid());
			if (!method_idx.IsValid()) {
				panicf("type[%d:%s] doesn't have method[%s] of type[%d:%s]", obj_tid, GET_TYPENAME_C(obj_tid), m_attr_name.c_str(), vc_param.GetResultTid(), GET_TYPENAME_C(vc_param.GetResultTid()));
			}
			m_method_idx	= method_idx;
			m_result_typeid = ti->GetMethodByIdx(method_idx)->GetTypeId();
		} else {
			// 上下文不足无法推断. 最后尝试下只用方法名查找, 如果有多个重名方法, 则失败
			MethodIndex method_idx = ti->GetMethodIdx(m_attr_name);
			if (!method_idx.IsValid()) {
				panicf("type[%d:%s] doesn't have method[%s]", obj_tid, GET_TYPENAME_C(obj_tid), m_attr_name.c_str());
			}
			m_method_idx	= method_idx;
			m_result_typeid = ti->GetMethodByIdx(method_idx)->GetTypeId();
		}
	}
	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeAccessAttr::Execute(ExecuteContext& ctx) {
	Variable* v = m_obj_expr->Execute(ctx);
	if (m_is_field) {
		panicf("not implemented");
	} else {
		return v->GetMethodValue(m_method_idx);
	}
}
