#include "type_type.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "type_mgr.h"
#include "verify_context.h"

TypeInfoType::TypeInfoType() {
	m_original_name = "type";
	m_is_value_type = true;
}
void TypeInfoType::InitBuiltinMethods(VerifyContext& ctx) {
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
					another_value_type->InitWithIdentifier("type");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("equal", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("Equal")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_TYPE}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	// 增加内置方法
	//{
	//	std::vector<AstNodeComplexFnDef*> fns;
	//	// 增加GetTypeName()str
	//	{
	//		std::vector<AstNodeComplexFnDef::Implement> implements;
	//		{
	//			std::vector<ParserGenericParam> gparams;
	//			std::vector<ParserParameter>	params;
	//			AstNodeType*					return_type = new AstNodeType();
	//			return_type->InitWithIdentifier("str");
	//			implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_getTypeName_verify));
	//		}
	//		AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("GetTypeName", implements);
	//		astnode_complex_fndef->Verify(ctx, VerifyContextParam());
	//		fns.push_back(astnode_complex_fndef);
	//	}
	//	// 增加GetTypeId()int
	//}
	ctx.PopStack();
}
