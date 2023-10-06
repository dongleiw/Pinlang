#include "type_float.h"

#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

#include <cassert>

TypeInfoFloat::TypeInfoFloat(TypeId tid) {
	m_typegroup_id	= TYPE_GROUP_ID_PRIMARY;
	m_is_value_type = true;
	switch (tid) {
	case TYPE_ID_FLOAT32:
		m_original_name = "f32";
		break;
	case TYPE_ID_FLOAT64:
		m_original_name = "f64";
		break;
	default:
		panicf("unexpected type[%d:%s]", tid, GET_TYPENAME_C(tid));
		break;
	}
}
void TypeInfoFloat::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	// 手动实现Add约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithTargetTypeId(m_typeid);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}

				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithTargetTypeId(m_typeid);

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}

			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());

			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		ConstraintInstance constraint_instance= constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	// 手动实现GreaterThan约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithTargetTypeId(m_typeid);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithTargetTypeId(TYPE_ID_BOOL);
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterThan", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("GreaterThan")->GetValueConstraint();
		ConstraintInstance constraint_instance= constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	ctx.PopStack();
}
llvm::Type* TypeInfoFloat::GetLLVMIRType(CompileContext& cctx) {
	switch (m_typeid) {
	case TYPE_ID_FLOAT32:
		return llvm::Type::getFloatTy(IRC);
		break;
	case TYPE_ID_FLOAT64:
		return llvm::Type::getDoubleTy(IRC);
		break;
	default:
		panicf("unexpected type[%d:%s]", m_typeid, GET_TYPENAME_C(m_typeid));
		break;
	}
}
