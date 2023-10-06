#include "type_bool.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "type_mgr.h"
#include "variable.h"
#include "verify_context.h"

#include <cassert>

TypeInfoBool::TypeInfoBool() {
	m_original_name	 = "bool";
	m_typegroup_id	 = TYPE_GROUP_ID_PRIMARY;
	m_is_value_type	 = true;
}
void TypeInfoBool::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	// 手动实现Equal约束
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("equal", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Equal")->GetValueConstraint();
		ConstraintInstance constraint_instance= constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	// 手动实现NotEqual约束
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("notEqual", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("NotEqual")->GetValueConstraint();
		ConstraintInstance constraint_instance= constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	ctx.PopStack();
}
llvm::Type* TypeInfoBool::GetLLVMIRType(CompileContext& cctx) {
	return llvm::Type::getInt1Ty(IRC);
}
