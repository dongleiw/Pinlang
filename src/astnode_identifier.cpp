#include "astnode_identifier.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "instruction.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"

AstNodeIdentifier::AstNodeIdentifier(std::string id) : m_id(id), m_is_complex_fn(false) {
}
VerifyContextResult AstNodeIdentifier::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	verify_begin();
	log_debug("verify identifier[%s]", m_id.c_str());

	Variable* v = ctx.GetCurStack()->GetVariableOrNull(m_id);

	if (v == nullptr) {
		log_info("unknown id[%s]. try to search and verify it first", m_id.c_str());
		ctx.GetGlobalBlock()->VerifyIdentfier(this, m_id, ctx, VerifyContextParam());
		v = ctx.GetCurStack()->GetVariableOrNull(m_id);
		if (v == nullptr) {
			panicf("unknown id[%s]", m_id.c_str());
		}
	}

	if (v->GetTypeId() == TYPE_ID_COMPLEX_FN) {
		/*
		 * 该标识符是一个函数的名字, 可以在verify阶段确定函数地址(即静态分发)
		 * 注意如果是普通的函数变量, 无法在verify阶段确定函数地址
		 */
		m_is_complex_fn = true;
		// 这是一个复杂函数, 需要根据上下文实例化
		AstNodeComplexFnDef* astnode_complex_fndef = v->GetValueComplexFn();
		if (vparam.HasFnCallArgs()) {
			// 父节点传递过来了函数调用的参数类型
			// 这说明该变量是函数
			// 根据参数类型和结果类型来实例化
			AstNodeComplexFnDef::Instance instance = astnode_complex_fndef->Instantiate_param_return(ctx, vparam.GetFnCallArgs(), vparam.GetResultTid());
			m_fn_addr							   = instance.fn_addr;
			m_fn_id								   = instance.instance_name;
			m_result_typeid						   = ctx.GetFnTable().GetFnTypeId(m_fn_addr);
		} else if (vparam.GetResultTid() != TYPE_ID_INFER) {
			// 父节点传递过来了期望的结果类型
			// 使用该类型来选择合适的函数重载
			AstNodeComplexFnDef::Instance instance = astnode_complex_fndef->Instantiate_type(ctx, vparam.GetResultTid());
			m_fn_addr							   = instance.fn_addr;
			m_fn_id								   = instance.instance_name;
			m_result_typeid						   = ctx.GetFnTable().GetFnTypeId(m_fn_addr);

		} else {
			// 上下文不足无法推断. 最后尝试下只用方法名查找, 如果有多个重名方法, 则失败
			AstNodeComplexFnDef::Instance instance = astnode_complex_fndef->Instantiate(ctx);
			m_fn_addr							   = instance.fn_addr;
			m_fn_id								   = instance.instance_name;
			m_result_typeid						   = ctx.GetFnTable().GetFnTypeId(m_fn_addr);
		}
	} else {
		m_is_complex_fn = false;
		m_result_typeid = v->GetTypeId();
	}

	m_compile_to_left_value = vparam.ExpectLeftValue();

	verify_end();
	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeIdentifier::Execute(ExecuteContext& ctx) {
	if (ctx.IsAssign()) {
		if (m_is_complex_fn) {
			panicf("complex fn can not be assigned");
		} else {
			ctx.GetCurStack()->GetVariable(m_id)->Assign(ctx.GetAssignValue());
		}
		return nullptr;
	} else {
		if (m_is_complex_fn) {
			return new Variable(m_result_typeid, FunctionObj(nullptr, m_fn_addr));
		} else {
			return ctx.GetCurStack()->GetVariable(m_id);
		}
	}
}
AstNodeIdentifier* AstNodeIdentifier::DeepCloneT() {
	return new AstNodeIdentifier(m_id);
}
llvm::Value* AstNodeIdentifier::Compile(CompileContext& cctx) {
	if (m_is_complex_fn) {
		// 是一个静态函数 (目前只支持静态函数)
		// 返回函数id
		if (!cctx.HasNamedValue(m_fn_id)) {
			panicf("fn[%s] not defined", m_fn_id.c_str());
		}
		return cctx.GetNamedValue(m_fn_id);
	} else {
		if (m_compile_to_left_value) {
			return cctx.GetNamedValue(m_id);
		} else {
			llvm::Value* v = cctx.GetNamedValue(m_id);

			TypeInfo*	ti		= g_typemgr.GetTypeInfo(m_result_typeid);
			llvm::Type* ir_type = ti->GetLLVMIRType(cctx);

			if (v->getType() == ir_type) {
				return v;
			} else if (v->getType() == ir_type->getPointerTo()) {
				return IRB.CreateLoad(ti->GetLLVMIRType(cctx), v);
			} else {
				panicf("bug");
			}
		}
	}
}
