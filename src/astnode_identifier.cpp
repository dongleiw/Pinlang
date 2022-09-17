#include "astnode_identifier.h"
#include "astnode_generic_fndef.h"
#include "define.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"

VerifyContextResult AstNodeIdentifier::Verify(VerifyContext& ctx) {
	log_debug("verify identifier[%s]", m_id.c_str());

	Variable*	   v		 = ctx.GetCurStack()->GetVariableOrNull(m_id);
	VariableTable* fn_def_vt = ctx.GetCurStack()->GetVariableTableByFnName(m_id);

	if (v != nullptr && fn_def_vt != nullptr) {
		panicf("bug");
	}
	if (v != nullptr) {
		if (v->GetTypeId() == TYPE_ID_GENERIC_FN) {
			// 这是一个泛型函数, 需要根据上下文实例化
			AstNodeGenericFnDef*	 astnode_generic_fndef = v->GetValueGenericFnDef();
			const VerifyContextParam vc_param			   = ctx.GetParam();
			if (vc_param.HasFnCallArgs()) {
				// 父节点传递过来了函数调用的参数类型
				// 这说明该变量是函数
				// 根据参数类型和结果类型来实例化
				std::string uniq_fnname = astnode_generic_fndef->Instantiate(ctx, vc_param.GetFnCallArgs(), vc_param.GetResultTid());
				Variable*	v			= ctx.GetCurStack()->GetVariableOrNull(uniq_fnname);
				if (v == nullptr) {
					panicf("var[%s] uniq_fnname[%s] not exist", m_id.c_str(), uniq_fnname.c_str());
				}
				log_info("change varname[%s] => [%s]", m_id.c_str(), uniq_fnname.c_str());
				m_id			= uniq_fnname;
				m_result_typeid = v->GetTypeId();
			} else if (vc_param.GetResultTid() != TYPE_ID_INFER) {
				// 父节点传递过来了期望的结果类型
				// 使用该类型来选择合适的函数重载
			} else {
				panicf("bug");
			}
		} else {
			m_result_typeid = v->GetTypeId();
		}
	} else if (fn_def_vt != nullptr) {
		// 是函数名, 需要根据上下文选择合适的重载实例
		const VerifyContextParam vc_param = ctx.GetParam();
		if (vc_param.HasFnCallArgs()) {
			// 父节点传递过来了函数调用的参数类型
			// 这说明该变量是函数
			// 根据参数类型来选择合适的函数重载
			std::string uniq_fnname = TypeInfoFn::GetUniqFnName(m_id, vc_param.GetFnCallArgs());
			Variable*	v			= ctx.GetCurStack()->GetVariableOrNull(uniq_fnname);
			if (v == nullptr) {
				panicf("var[%s] uniq_fnname[%s] not exist", m_id.c_str(), uniq_fnname.c_str());
			}
			log_info("change varname[%s] => [%s]", m_id.c_str(), uniq_fnname.c_str());
			m_id			= uniq_fnname;
			m_result_typeid = v->GetTypeId();
		} else if (vc_param.GetResultTid() != TYPE_ID_INFER) {
			// 父节点传递过来了期望的结果类型
			// 使用该类型来选择合适的函数重载
			TypeId		expect_result_tid = vc_param.GetResultTid();
			TypeInfoFn* tifn			  = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(expect_result_tid));
			std::string uniq_fnname		  = m_id + "_" + TypeInfoFn::GetUniqFnName(m_id, tifn->GetParmsTid());
			Variable*	v				  = ctx.GetCurStack()->GetVariableOrNull(uniq_fnname);
			if (v == nullptr) {
				panicf("var[%s] uniq_fnname[%s] not exist", m_id.c_str(), uniq_fnname.c_str());
			}
			log_info("change varname[%s] => [%s]", m_id.c_str(), uniq_fnname.c_str());
			m_id			= uniq_fnname;
			m_result_typeid = expect_result_tid;
		} else {
			panicf("bug");
		}
	} else {
		panicf("unknown id[%s]", m_id.c_str());
	}

	return VerifyContextResult(m_result_typeid);
}
Variable* AstNodeIdentifier::Execute(ExecuteContext& ctx) {
	return ctx.GetCurStack()->GetVariable(m_id);
}
