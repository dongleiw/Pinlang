#include "astnode_type_cast.h"
#include "astnode_compile_const.h"
#include "astnode_type.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "verify_context.h"
#include <llvm-12/llvm/IR/Type.h>
#include <llvm-12/llvm/IR/Value.h>

AstNodeTypeCast::AstNodeTypeCast(AstNode* expr, AstNodeType* dst_type) {
	m_expr	   = expr;
	m_dst_type = dst_type;

	m_expr->SetParent(this);
	m_dst_type->SetParent(this);
}

/*
 * 调用函数
 */
VerifyContextResult AstNodeTypeCast::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	VerifyContextResult vr_expr		= m_expr->Verify(ctx, VerifyContextParam());
	VerifyContextResult vr_dst_type = m_dst_type->Verify(ctx, VerifyContextParam());

	if (vr_expr.GetResultTypeId() == vr_dst_type.GetResultTypeId()) {
		panicf("cast to the same type[%d:%s]", vr_dst_type.GetResultTypeId(), GET_TYPENAME_C(vr_dst_type.GetResultTypeId()));
	}

	m_src_tid = vr_expr.GetResultTypeId();
	m_dst_tid = vr_dst_type.GetResultTypeId();

	if (is_integer_type(m_src_tid)) {
		if (is_integer_type(m_dst_tid)) {
			// integer => integer
		} else if (is_float_type(m_dst_tid)) {
			// integer => float
		} else {
			panicf("type[%d:%s] => type[%d:%s] not support", m_src_tid, GET_TYPENAME_C(m_src_tid),
				   m_dst_tid, GET_TYPENAME_C(m_dst_tid));
		}
	} else {
		panicf("type[%d:%s] => type[%d:%s] not support", m_src_tid, GET_TYPENAME_C(m_src_tid),
			   m_dst_tid, GET_TYPENAME_C(m_dst_tid));
	}

	VerifyContextResult vr;
	vr.SetResultTypeId(vr_dst_type.GetResultTypeId());
	return vr;
}
AstNodeTypeCast* AstNodeTypeCast::DeepCloneT() {
	AstNodeTypeCast* newone = new AstNodeTypeCast(m_expr->DeepClone(), m_dst_type->DeepCloneT());
	newone->Copy(*this);
	return newone;
}
CompileResult AstNodeTypeCast::Compile(CompileContext& cctx) {
	TypeInfo* ti_src = g_typemgr.GetTypeInfo(m_src_tid);
	TypeInfo* ti_dst = g_typemgr.GetTypeInfo(m_dst_tid);

	CompileResult cr_src_value = m_expr->Compile(cctx);

	llvm::Value* result = nullptr;

	if (is_integer_type(m_src_tid)) {
		if (is_integer_type(m_dst_tid)) {
			if (is_unsigned_integer_type(m_src_tid)) {
				if (get_integer_bits(m_src_tid) < get_integer_bits(m_dst_tid)) {
					// unsigned upercast
					result = IRB.CreateZExt(cr_src_value.GetResult(), ti_dst->GetLLVMIRType(cctx));
				} else if (get_integer_bits(m_src_tid) > get_integer_bits(m_dst_tid)) {
					// unsigned downcast
					result = IRB.CreateTrunc(cr_src_value.GetResult(), ti_dst->GetLLVMIRType(cctx));
				} else {
					// bitcast
					result = IRB.CreateBitCast(cr_src_value.GetResult(), ti_dst->GetLLVMIRType(cctx));
				}
			} else {
				if (get_integer_bits(m_src_tid) < get_integer_bits(m_dst_tid)) {
					// signed upercast
					result = IRB.CreateSExt(cr_src_value.GetResult(), ti_dst->GetLLVMIRType(cctx));
				} else if (get_integer_bits(m_src_tid) > get_integer_bits(m_dst_tid)) {
					// signed downcast
					result = IRB.CreateTrunc(cr_src_value.GetResult(), ti_dst->GetLLVMIRType(cctx));
				} else {
					// bitcast
					result = IRB.CreateBitCast(cr_src_value.GetResult(), ti_dst->GetLLVMIRType(cctx));
				}
			}
		} else if (is_float_type(m_dst_tid)) {
			// integer => float
			panicf("type[%d:%s] => type[%d:%s] not support", m_src_tid, GET_TYPENAME_C(m_src_tid),
				   m_dst_tid, GET_TYPENAME_C(m_dst_tid));
		} else {
			panicf("type[%d:%s] => type[%d:%s] not support", m_src_tid, GET_TYPENAME_C(m_src_tid),
				   m_dst_tid, GET_TYPENAME_C(m_dst_tid));
		}
	} else {
		panicf("type[%d:%s] => type[%d:%s] not support", m_src_tid, GET_TYPENAME_C(m_src_tid),
			   m_dst_tid, GET_TYPENAME_C(m_dst_tid));
	}

	return CompileResult().SetResult(result);
}
