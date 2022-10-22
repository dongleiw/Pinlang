#include "astnode_identifier.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"

VerifyContextResult AstNodeIdentifier::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify identifier[%s]", m_id.c_str());

	Variable* v = ctx.GetCurStack()->GetVariableOrNull(m_id);

	if (v != nullptr) {
		if (v->GetTypeId() == TYPE_ID_COMPLEX_FN) {
			// 这是一个复杂函数, 需要根据上下文实例化
			AstNodeComplexFnDef* astnode_complex_fndef = v->GetValueComplexFn();
			if (vparam.HasFnCallArgs()) {
				// 父节点传递过来了函数调用的参数类型
				// 这说明该变量是函数
				// 根据参数类型和结果类型来实例化
				AstNodeComplexFnDef::Instance instance = astnode_complex_fndef->Instantiate_param_return(ctx, vparam.GetFnCallArgs(), vparam.GetResultTid());
				log_info("change varname[%s] => [%s]", m_id.c_str(), instance.instance_name.c_str());
				m_id			= instance.instance_name;
				m_result_typeid = instance.fnobj.GetFunction()->GetTypeId();
			} else if (vparam.GetResultTid() != TYPE_ID_INFER) {
				// 父节点传递过来了期望的结果类型
				// 使用该类型来选择合适的函数重载
				AstNodeComplexFnDef::Instance instance = astnode_complex_fndef->Instantiate_type(ctx, vparam.GetResultTid());
				log_info("change varname[%s] => [%s]", m_id.c_str(), instance.instance_name.c_str());
				m_id			= instance.instance_name;
				m_result_typeid = instance.fnobj.GetFunction()->GetTypeId();

			} else {
				panicf("bug");
			}
		} else {
			m_result_typeid = v->GetTypeId();
		}
	} else {
		panicf("unknown id[%s]", m_id.c_str());
	}

	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeIdentifier::Execute(ExecuteContext& ctx) {
	if (ctx.IsAssign()) {
		ctx.GetCurStack()->GetVariable(m_id)->Assign(ctx.GetAssignValue());
		return nullptr;
	} else {
		return ctx.GetCurStack()->GetVariable(m_id);
	}
}
AstNodeIdentifier* AstNodeIdentifier::DeepCloneT() {
	return new AstNodeIdentifier(m_id);
}
