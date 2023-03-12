#include "astnode_access_attr.h"
#include "astnode_fncall.h"
#include "compile_context.h"
#include "define.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include "log.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/Function.h>
#include <llvm-12/llvm/Support/Casting.h>

AstNodeAccessAttr::AstNodeAccessAttr(AstNode* obj_expr, std::string attr_name) {
	m_obj_expr	= obj_expr;
	m_attr_name = attr_name;
	m_obj_expr->SetParent(this);
}
VerifyContextResult AstNodeAccessAttr::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	log_debug("verify access attr[%s]", m_attr_name.c_str());

	VerifyContextResult vr_expr = m_obj_expr->Verify(ctx, VerifyContextParam().SetExepectLeftValue(true));
	m_obj_tid					= vr_expr.GetResultTypeId();
	TypeInfo* ti				= g_typemgr.GetTypeInfo(m_obj_tid);
	if (ti->HasField(m_attr_name)) {
		// 是字段
		m_is_field		= true;
		m_result_typeid = ti->GetFieldType(m_attr_name);
	} else {
		// 是方法
		m_is_field = false;
		if (vparam.HasFnCallArgs()) {
			// 父节点传递过来了函数调用的参数类型
			// 根据参数类型和结果类型来选择
			std::vector<std::string> method_idxs = ti->GetConstraintMethod(ctx, "", m_attr_name, vparam.GetFnCallArgs(), TYPE_ID_INFER);
			if (method_idxs.empty()) {
				panicf("type[%d:%s] doesn't have method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			if (method_idxs.size() > 1) {
				panicf("type[%d:%s] has multiple method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			std::string fnid = method_idxs.at(0);
			m_result_typeid	 = ctx.GetFnTable().GetFnTypeId(fnid);
			m_fnid			 = ctx.GetFnTable().GetFnTypeId(fnid);
		} else if (vparam.GetResultTid() != TYPE_ID_INFER) {
			// 父节点传递过来了期望的结果类型
			// 使用该类型来选择合适的函数重载
			std::vector<std::string> method_idxs = ti->GetConstraintMethod(ctx, "", m_attr_name, vparam.GetResultTid());
			if (method_idxs.empty()) {
				panicf("type[%d:%s] doesn't have method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			if (method_idxs.size() > 1) {
				panicf("type[%d:%s] has multiple method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			std::string fnid = method_idxs.at(0);
			m_result_typeid	 = ctx.GetFnTable().GetFnTypeId(fnid);
			m_fnid			 = ctx.GetFnTable().GetFnTypeId(fnid);
		} else {
			// 上下文不足无法推断. 最后尝试下只用方法名查找, 如果有多个重名方法, 则失败
			std::vector<std::string> method_idxs = ti->GetConstraintMethod(ctx, "", m_attr_name);
			if (method_idxs.empty()) {
				panicf("type[%d:%s] doesn't have method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			if (method_idxs.size() > 1) {
				panicf("type[%d:%s] has multiple method[%s] with args[%s]", m_obj_tid, GET_TYPENAME_C(m_obj_tid), m_attr_name.c_str(), g_typemgr.GetTypeName(vparam.GetFnCallArgs()).c_str());
			}
			std::string fnid = method_idxs.at(0);
			m_result_typeid	 = ctx.GetFnTable().GetFnTypeId(fnid);
			m_fnid			 = ctx.GetFnTable().GetFnTypeId(fnid);
		}
	}

	m_compile_to_left_value = vparam.ExpectLeftValue();
	return VerifyContextResult(m_result_typeid).SetTmp(false);
}
Variable* AstNodeAccessAttr::Execute(ExecuteContext& ctx) {
	Variable* assign_value = ctx.GetAssignValue();
	ctx.SetAssignValue(nullptr);
	if (assign_value != nullptr) {
		if (m_is_field) {
			Variable* v = m_obj_expr->Execute(ctx);
			v->SetFieldValue(m_attr_name, assign_value);
			return nullptr;
		} else {
			panicf("assign method!");
		}
	} else {
		Variable* v = m_obj_expr->Execute(ctx);
		if (m_is_field) {
			return v->GetFieldValue(m_attr_name);
		} else {
			Variable* method_v = new Variable(m_result_typeid, FunctionObj(v));
			return method_v;
		}
	}
}
AstNodeAccessAttr* AstNodeAccessAttr::DeepCloneT() {
	AstNodeAccessAttr* newone = new AstNodeAccessAttr(m_obj_expr->DeepClone(), m_attr_name);
	return newone;
}
CompileResult AstNodeAccessAttr::Compile(CompileContext& cctx) {
	CompileResult cr_obj = m_obj_expr->Compile(cctx);

	if (m_is_field) {
		TypeInfo*	ti_obj		= g_typemgr.GetTypeInfo(m_obj_tid);
		llvm::Type* ir_type_obj = ti_obj->GetLLVMIRType(cctx);
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

		return CompileResult().SetResultMethod((llvm::Function*)cctx.GetNamedValue(m_fnid), cr_obj.GetResult());
	}
}
