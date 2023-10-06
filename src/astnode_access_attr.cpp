#include "astnode_access_attr.h"
#include "astnode.h"
#include "astnode_fncall.h"
#include "astnode_identifier.h"
#include "compile_context.h"
#include "define.h"
#include "fntable.h"
#include "type.h"
#include "type_class.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_pointer.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/Support/Casting.h>

AstNodeAccessAttr::AstNodeAccessAttr(AstNode* obj_expr, std::string attr_name) {
	m_obj_expr	= obj_expr;
	m_attr_name = attr_name;
	m_obj_expr->SetParent(this);
	m_is_obj_expr_verified = false;
	m_obj_is_type		   = false;
}
bool AstNodeAccessAttr::IsSimpleFn(VerifyContext& ctx) {
	if (!m_is_obj_expr_verified) {
		AstNodeIdentifier* astnode_identifier = dynamic_cast<AstNodeIdentifier*>(m_obj_expr);
		if (astnode_identifier != nullptr) {
			// 如果语法`a.b`中,`a`的类型是type(包裹的类型为T), 那么实际获取的是T的类型
			m_vr_obj_expr = astnode_identifier->Verify_as_tid(ctx, m_obj_is_type);
		} else {
			m_vr_obj_expr = m_obj_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(false));
		}
		m_is_obj_expr_verified = true;
	}

	TypeInfo* ti = g_typemgr.GetTypeInfo(m_vr_obj_expr.GetResultTypeId());
	return ti->IsSimpleMethod(m_attr_name);
}
VerifyContextResult AstNodeAccessAttr::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VERIFY_BEGIN;

	log_debug("verify access attr[%s]", m_attr_name.c_str());

	VerifyContextResult vr;

	if (!m_is_obj_expr_verified) {
		m_vr_obj_expr		   = m_obj_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(false));
		m_is_obj_expr_verified = true;
	}
	m_obj_tid	 = m_vr_obj_expr.GetResultTypeId();
	TypeInfo* ti = g_typemgr.GetTypeInfo(m_obj_tid);
	if (ti->IsPointer()) {
		// 该语法同时支持值和pointer两种
		// 自动解引用
		ti = g_typemgr.GetTypeInfo(dynamic_cast<TypeInfoPointer*>(ti)->GetPointeeTid());
		//m_obj_expr->SetCompileToRightValue();
		m_obj_tid = ti->GetTypeId();
	} else {
		// 不是指针. 需要返回lvalue, 来获取内存地址
		m_obj_expr->SetCompileToLeftValue();
	}
	//if (!ti->IsClass()) {
	//	panicf("type[%d:%s] doesn't have attr[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str());
	//}
	TypeInfoClass* ti_class = dynamic_cast<TypeInfoClass*>(ti);
	if (ti_class != nullptr && ti_class->HasField(m_attr_name)) {
		// 是字段
		m_is_field		= true;
		m_result_typeid = ti_class->GetFieldType(m_attr_name);
	} else {
		// 是方法
		m_is_field = false;

		if (vparam.HasFnCallArgs()) {
			// 父节点传递过来了函数调用的参数类型
			// 根据参数类型和结果类型来选择
			std::vector<std::string> method_idxs = ti->GetConstraintMethod(ctx, m_attr_name, vparam.GetFnCallArgs(), TYPE_ID_INFER);
			if (method_idxs.empty()) {
				panicf("type[%d:%s] doesn't have method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			if (method_idxs.size() > 1) {
				panicf("type[%d:%s] has multiple method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			m_fnid = method_idxs.at(0);
		} else if (vparam.GetExpectResultTid() != TYPE_ID_INFER) {
			// 父节点传递过来了期望的结果类型
			// 使用该类型来选择合适的函数重载
			std::vector<std::string> method_idxs = ti->GetConstraintMethod(ctx, m_attr_name, vparam.GetExpectResultTid());
			if (method_idxs.empty()) {
				panicf("type[%d:%s] doesn't have method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			if (method_idxs.size() > 1) {
				panicf("type[%d:%s] has multiple method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			m_fnid = method_idxs.at(0);
		} else {
			// 上下文不足无法推断. 最后尝试下只用方法名查找, 如果有多个重名方法, 则失败
			std::vector<std::string> method_idxs = ti->GetConstraintMethod(ctx, m_attr_name);
			if (method_idxs.empty()) {
				panicf("type[%d:%s] doesn't have method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			if (method_idxs.size() > 1) {
				panicf("type[%d:%s] has multiple method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			m_fnid = method_idxs.at(0);
		}
		const FnInfo* fn_info = ctx.GetFnTable().GetFnInfo(m_fnid);
		m_result_typeid		  = fn_info->GetFnTid();
		assert(m_obj_tid == fn_info->GetObjTid());
		vr.SetObjTid(m_obj_tid);
		assert(fn_info->IsMethod());
		vr.SetIsMethod(true);
		vr.SetHasSelfParam(fn_info->HasSelfParam());
		vr.SetIsConstructor(fn_info->IsConstructor());
	}

	m_compile_to_left_value = vparam.ExpectLeftValue();
	vr.SetResultTypeId(m_result_typeid);
	vr.SetTmp(false);
	return vr;
}
Variable* AstNodeAccessAttr::Execute(ExecuteContext& ctx) {
	panicf("");
	return nullptr;
}
AstNodeAccessAttr* AstNodeAccessAttr::DeepCloneT() {
	AstNodeAccessAttr* newone = new AstNodeAccessAttr(m_obj_expr->DeepClone(), m_attr_name);
	newone->Copy(*this);
	return newone;
}
CompileResult AstNodeAccessAttr::Compile(CompileContext& cctx) {
	if (m_obj_is_type) {
		if (!cctx.HasNamedValue(m_fnid)) {
			panicf("fn[%s] not defined", m_fnid.c_str());
		}
		llvm::Value* v = cctx.GetNamedValue(m_fnid);
		assert(llvm::Function::classof(v));

		// 不管是lvalue还是rvalue, 方法都返回Function?
		return CompileResult().SetResultMethod((llvm::Function*)v, nullptr);
	} else {
		CompileResult cr_obj = m_obj_expr->Compile(cctx);

		if (m_is_field) {
			TypeInfoClass* ti_obj	   = dynamic_cast<TypeInfoClass*>(g_typemgr.GetTypeInfo(m_obj_tid));
			llvm::Type*	   ir_type_obj = ti_obj->GetLLVMIRType(cctx);
			assert(ir_type_obj->getPointerTo() == cr_obj.GetResult()->getType());
			llvm::Value* attr_value = IRB.CreateConstGEP2_32(ir_type_obj, cr_obj.GetResult(), 0, ti_obj->GetFieldIndex(m_attr_name));
			if (!m_compile_to_left_value) {
				TypeInfo* ti_field = g_typemgr.GetTypeInfo(m_result_typeid);
				attr_value		   = IRB.CreateLoad(ti_field->GetLLVMIRType(cctx), attr_value);
			}
			return CompileResult().SetResult(attr_value);
		} else {
			if (!cctx.HasNamedValue(m_fnid)) {
				panicf("fn[%s] not defined", m_fnid.c_str());
			}
			llvm::Value* v = cctx.GetNamedValue(m_fnid);
			assert(llvm::Function::classof(v));

			// 不管是lvalue还是rvalue, 方法都返回Function?
			return CompileResult().SetResultMethod((llvm::Function*)v, cr_obj.GetResult());
		}
	}
}
