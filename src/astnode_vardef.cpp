#include <cstdlib>
#include <ios>
#include <iterator>
#include <llvm-12/llvm/IR/Instructions.h>
#include <llvm-12/llvm/IR/Type.h>

#include "astnode_literal.h"
#include "astnode_type.h"
#include "astnode_vardef.h"
#include "compile_context.h"
#include "define.h"
#include "instruction.h"
#include "log.h"
#include "support/CPPUtils.h"
#include "type.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "verify_context.h"

AstNodeVarDef::AstNodeVarDef(std::string var_name, AstNodeType* declared_type, AstNode* init_expr, bool is_const) {
	m_varname		= var_name;
	m_declared_type = declared_type;
	m_init_expr		= init_expr;
	m_is_const		= is_const;

	if (m_init_expr != nullptr) {
		m_init_expr->SetParent(this);
	}
}
VerifyContextResult AstNodeVarDef::Verify(VerifyContext& ctx, VerifyContextParam vparam) {
	verify_begin();
	log_debug("verify vardef: varname[%s]", m_varname.c_str());

	TypeId declared_tid = TYPE_ID_INFER;
	if (m_declared_type != nullptr) {
		VerifyContextResult vr = m_declared_type->Verify(ctx, VerifyContextParam());
		declared_tid		   = vr.GetResultTypeId();
	}

	if (m_init_expr != nullptr) {
		const VerifyContextResult vr_init_expr = m_init_expr->Verify(ctx, VerifyContextParam().SetResultTid(declared_tid));
		if (vr_init_expr.GetResultTypeId() == TYPE_ID_NONE) {
			panicf("init expr should be expression");
		}
		if (declared_tid == TYPE_ID_INFER) {
		} else {
			if (declared_tid != vr_init_expr.GetResultTypeId()) {
				panicf("var[%s] declared type[%s] != init expr type[%s]", m_varname.c_str(), GET_TYPENAME_C(declared_tid),
					   GET_TYPENAME_C(vr_init_expr.GetResultTypeId()));
			}
		}
		m_result_typeid = vr_init_expr.GetResultTypeId();
	} else if (declared_tid != TYPE_ID_INFER) {
		m_result_typeid = declared_tid;
	} else {
		panicf("var[%s] need type", m_varname.c_str())
	}

	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_varname, new Variable(m_result_typeid));
	verify_end();
	return VerifyContextResult();
}
Variable* AstNodeVarDef::Execute(ExecuteContext& ctx) {
	Variable* v = new Variable(m_result_typeid);
	if (m_init_expr != nullptr) {
		v->Assign(m_init_expr->Execute(ctx));
	}
	ctx.GetCurStack()->GetCurVariableTable()->AddVariable(m_varname, v);
	return nullptr;
}
AstNodeVarDef* AstNodeVarDef::DeepCloneT() {
	AstNodeVarDef* newone = new AstNodeVarDef();

	newone->m_varname = m_varname;
	if (m_declared_type != nullptr)
		newone->m_declared_type = m_declared_type->DeepCloneT();
	if (m_init_expr != nullptr)
		newone->m_init_expr = m_init_expr->DeepClone();
	newone->m_is_const = m_is_const;

	return newone;
}
CompileResult AstNodeVarDef::Compile(CompileContext& cctx) {
	TypeInfo*		  ti		  = g_typemgr.GetTypeInfo(m_result_typeid);
	llvm::Type*		  ir_type	  = ti->GetLLVMIRType(cctx);
	llvm::AllocaInst* alloca_inst = IRB.CreateAlloca(ir_type, nullptr, m_varname);

	cctx.AddNamedValue(m_varname, alloca_inst);

	if (m_init_expr != nullptr) {
		CompileResult cr_init_value = m_init_expr->Compile(cctx);
		assert(cr_init_value.GetResult()->getType() == ir_type);
		IRB.CreateStore(cr_init_value.GetResult(), alloca_inst);
	}
	return CompileResult().SetResult(alloca_inst);
}
