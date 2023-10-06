#include "astnode_identifier.h"
#include "astnode.h"
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
	m_ast_type = AstType::IDENTIFIER;
}
bool AstNodeIdentifier::IsSimpleFn(VerifyContext& vctx) {
	log_debug("check if identifier[%s] is simple fn", m_id.c_str());

	Variable* v = vctx.GetCurStack()->GetVariableOrNull(m_id);

	if (v == nullptr) {
		log_info("unknown id[%s]. try to search and verify it first", m_id.c_str());
		vctx.VerifyGlobalIdentifier(this, m_id, VerifyContextParam());
		v = vctx.GetCurStack()->GetVariableOrNull(m_id);
		if (v == nullptr) {
			panicf("unknown id[%s]", m_id.c_str());
		}
	}

	if (v->GetTypeId() == TYPE_ID_COMPLEX_FN) {
		AstNodeComplexFnDef* astnode_complex_fndef = v->GetValueComplexFn();
		if (astnode_complex_fndef->IsSimpleFn()) {
			// 这是一个简单函数(无重载,非泛型). 因此函数的类型是明确的
			return true;
		}
	} else if (v->GetTypeId() == TYPE_ID_TYPE) {
		// 是一个type类型, 如果作为一个函数调用. 需要查看同名构造函数是否是simplefn
		TypeInfo* ti_class = g_typemgr.GetTypeInfo(v->GetValueTid());
		if (ti_class->IsSimpleConstrcutor(ti_class->GetOriginalName())) {
			return true;
		}
	}
	return false;
}
VerifyContextResult AstNodeIdentifier::Verify_as_tid(VerifyContext& ctx, bool& is_type) {
	VERIFY_BEGIN;
	log_debug("verify identifier[%s]", m_id.c_str());

	VerifyContextResult vr;

	Variable* v = ctx.GetCurStack()->GetVariableOrNull(m_id);

	if (v == nullptr) {
		log_info("unknown id[%s]. try to search and verify it first", m_id.c_str());
		ctx.VerifyGlobalIdentifier(this, m_id, VerifyContextParam());
		v = ctx.GetCurStack()->GetVariableOrNull(m_id);
		if (v == nullptr) {
			panicf("unknown id[%s]", m_id.c_str());
		}
	}
	m_is_complex_fn = false;
	if (v->GetTypeId() == TYPE_ID_TYPE) {
		// 如果语法`a.b`中,`a`的类型是type(包裹的类型为T), 那么实际获取的是T的类型
		m_result_typeid = v->GetValueTid();
		is_type			= true;
	} else {
		m_result_typeid = v->GetTypeId();
		is_type			= false;
	}

	vr.SetTmp(false);
	vr.SetResultTypeId(m_result_typeid);
	return vr;
}
VerifyContextResult AstNodeIdentifier::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;
	log_debug("verify identifier[%s]", m_id.c_str());

	VerifyContextResult vr;

	Variable* v = ctx.GetCurStack()->GetVariableOrNull(m_id);

	if (v == nullptr) {
		log_info("unknown id[%s]. try to search and verify it first", m_id.c_str());
		ctx.VerifyGlobalIdentifier(this, m_id, VerifyContextParam());
		v = ctx.GetCurStack()->GetVariableOrNull(m_id);
		if (v == nullptr) {
			panicf("unknown id[%s]", m_id.c_str());
		}
	}

	if (vparam.GetExpectResultTgid() == TYPE_GROUP_ID_FUNCTION) {
		if (v->GetTypeId() == TYPE_ID_COMPLEX_FN) {
			/*
		 * 该标识符是一个函数的名字, 可以在verify阶段确定函数地址(即静态分发)
		 * 注意如果是普通的函数变量, 无法在verify阶段确定函数地址
		 */
			m_is_complex_fn							   = true;
			AstNodeComplexFnDef* astnode_complex_fndef = v->GetValueComplexFn();
			// 这是一个复杂函数, 需要根据上下文实例化
			if (vparam.HasFnCallArgs()) {
				// 父节点传递过来了函数调用的参数类型
				// 根据参数类型和结果类型来实例化
				m_fn_id			= astnode_complex_fndef->Instantiate_param_return(ctx, vparam.GetFnCallArgs(), vparam.GetExpectFnReturnTid());
				m_result_typeid = ctx.GetFnTable().GetFnTypeId(m_fn_id);
			} else if (vparam.GetExpectResultTid() != TYPE_ID_INFER) {
				// 父节点传递过来了期望的结果类型
				// 使用该类型来选择合适的函数重载
				m_fn_id			= astnode_complex_fndef->Instantiate_type(ctx, vparam.GetExpectResultTid());
				m_result_typeid = ctx.GetFnTable().GetFnTypeId(m_fn_id);
			} else {
				// 上下文不足无法推断. 最后尝试下只用方法名查找, 如果有多个重名方法, 则失败
				m_fn_id			= astnode_complex_fndef->Instantiate(ctx);
				m_result_typeid = ctx.GetFnTable().GetFnTypeId(m_fn_id);
			}
		} else if (v->GetTypeId() == TYPE_ID_TYPE) {
			// 是一个type类型, parent-node传递进来了函数调用的参数. 因此这是一个构造函数的调用
			m_is_complex_fn = true;
			vr.SetIsConstructor(true);
			vr.SetHasSelfParam(true);
			vr.SetIsMethod(true);
			vr.SetObjTid(v->GetValueTid());

			TypeInfo*				 ti_type			= g_typemgr.GetTypeInfo(v->GetValueTid());
			std::string				 constructor_fnname = m_id;
			std::vector<std::string> constructors;
			if (vparam.HasFnCallArgs()) {
				constructors = ti_type->GetConstructor(ctx, constructor_fnname, vparam.GetFnCallArgs());
			} else {
				constructors = ti_type->GetConstructor(ctx, constructor_fnname);
			}
			if (constructors.empty()) {
				panicf("no candidate constructor match");
			} else if (constructors.size() > 1) {
				panicf("multiple candidates constructor match");
			} else {
				m_fn_id			= constructors.at(0);
				m_result_typeid = ctx.GetFnTable().GetFnTypeId(m_fn_id);
			}
		} else {
			panicf("unknown type[%d:%s] of var[%s]", v->GetTypeId(), GET_TYPENAME_C(v->GetTypeId()), m_id.c_str());
		}
	} else {
		m_is_complex_fn = false;
		m_result_typeid = v->GetTypeId();
	}

	m_compile_to_left_value = vparam.ExpectLeftValue();

	vr.SetTmp(false);
	vr.SetResultTypeId(m_result_typeid);
	return vr;
}
Variable* AstNodeIdentifier::Execute(ExecuteContext& ctx) {
	panicf("");
	return nullptr;
}
AstNodeIdentifier* AstNodeIdentifier::DeepCloneT() {
	AstNodeIdentifier* newone = new AstNodeIdentifier(m_id);
	newone->Copy(*this);
	return newone;
}
CompileResult AstNodeIdentifier::Compile(CompileContext& cctx) {
	if (m_is_complex_fn) {
		// 是一个静态函数 (目前只支持静态函数)
		if (!cctx.HasNamedValue(m_fn_id)) {
			panicf("fn[%s] not defined", m_fn_id.c_str());
		}
		llvm::Value* f = cctx.GetNamedValue(m_fn_id);
		// 不管是rvalue还是lvalue, 都返回函数?
		if (m_compile_to_left_value) {
			assert(llvm::Function::classof(f));
			return CompileResult().SetResultFn((llvm::Function*)f);
		} else {
			assert(llvm::Function::classof(f));
			return CompileResult().SetResultFn((llvm::Function*)f);
		}
	} else {
		TypeInfo*	 ti		 = g_typemgr.GetTypeInfo(m_result_typeid);
		llvm::Type*	 ir_type = ti->GetLLVMIRType(cctx);
		llvm::Value* v		 = cctx.GetNamedValue(m_id);

		// named value map中存储的可能是一个指向value的指针, 也可能是一个register value
		// 例如分配在栈上的变量就是一个pointer, 而参数传递过来的值就是register value
		// 目前所有函数的参数(除this外)都进行了rvalue=>lvalue的处理.
		llvm::Value* result_v = nullptr;
		if (m_id == "self") {
			assert(v->getType() == ir_type);
			if (m_compile_to_left_value) {
				panicf("can not access lvalue of self")
			} else {
				result_v = v;
			}
		} else {
			assert(v->getType() == ir_type->getPointerTo());
			if (m_compile_to_left_value) {
				result_v = v;
			} else {
				result_v = IRB.CreateLoad(ir_type, v);
			}
		}
		return CompileResult().SetResult(result_v);
	}
}
void AstNodeIdentifier::ClearVerifyState() {
	AstNode::ClearVerifyState();
	m_fn_id.clear();
	m_is_complex_fn = false;
}
