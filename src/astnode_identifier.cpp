#include "astnode_identifier.h"
#include "astnode_complex_fndef.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/Support/Casting.h>

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
			m_fn_id								   = instance.instance_name;
			m_result_typeid						   = ctx.GetFnTable().GetFnTypeId(m_fn_id);
		} else if (vparam.GetResultTid() != TYPE_ID_INFER) {
			// 父节点传递过来了期望的结果类型
			// 使用该类型来选择合适的函数重载
			AstNodeComplexFnDef::Instance instance = astnode_complex_fndef->Instantiate_type(ctx, vparam.GetResultTid());
			m_fn_id								   = instance.instance_name;
			m_result_typeid						   = ctx.GetFnTable().GetFnTypeId(m_fn_id);

		} else {
			// 上下文不足无法推断. 最后尝试下只用方法名查找, 如果有多个重名方法, 则失败
			AstNodeComplexFnDef::Instance instance = astnode_complex_fndef->Instantiate(ctx);
			m_fn_id								   = instance.instance_name;
			m_result_typeid						   = ctx.GetFnTable().GetFnTypeId(m_fn_id);
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
			return new Variable(m_result_typeid, FunctionObj(nullptr));
		} else {
			return ctx.GetCurStack()->GetVariable(m_id);
		}
	}
}
AstNodeIdentifier* AstNodeIdentifier::DeepCloneT() {
	return new AstNodeIdentifier(m_id);
}
CompileResult AstNodeIdentifier::Compile(CompileContext& cctx) {
	if (m_is_complex_fn) {
		// 是一个静态函数 (目前只支持静态函数)
		// 返回函数id
		if (!cctx.HasNamedValue(m_fn_id)) {
			panicf("fn[%s] not defined", m_fn_id.c_str());
		}
		llvm::Value* f = cctx.GetNamedValue(m_fn_id);
		assert(llvm::Function::classof(f));
		return CompileResult().SetResultFn((llvm::Function*)f);
	} else {
		TypeInfo*	 ti		 = g_typemgr.GetTypeInfo(m_result_typeid);
		llvm::Type*	 ir_type = ti->GetLLVMIRType(cctx);
		llvm::Value* v		 = cctx.GetNamedValue(m_id);

		// named value map中存储的可能是一个指向value的指针, 也可能是一个register value
		// 例如分配在栈上的变量就是一个pointer, 而参数传递过来的值就是register value
		llvm::Value* result_v = nullptr;
		if (v->getType() == ir_type) {
			if (m_compile_to_left_value) {
				// 当前变量是一个value, 但是parent-node要求返回lvalue. 这里需要先store到栈内存, 然后获取内存地址返回
				llvm::Value* ptr = IRB.CreateAlloca(ir_type);
				IRB.CreateStore(v, ptr);
				// 使用ptr替换掉named value map中的value. 后续获取的变量都是ptr. ptr中的值可能被修改, 不替换的话会导致获取的可能是旧值
				cctx.ReplaceNamedValue(m_id, ptr);
				result_v = ptr;
			} else {
				result_v = v;
			}
		} else if (v->getType() == ir_type->getPointerTo()) {
			if (m_compile_to_left_value) {
				result_v = v;
			} else {
				result_v = IRB.CreateLoad(ir_type, v);
			}
		} else {
			panicf("bug");
		}
		return CompileResult().SetResult(result_v);
	}
}
