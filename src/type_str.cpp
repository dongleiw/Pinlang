#include "type_str.h"

#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "execute_context.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"

#include <cassert>

TypeInfoStr::TypeInfoStr() {
	m_name			 = "str";
	m_typegroup_id	 = TYPE_GROUP_ID_PRIMARY;
}
void TypeInfoStr::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	// 手动实现Add约束
	//{
	//	std::vector<AstNodeComplexFnDef*> fns;
	//	{
	//		std::vector<AstNodeComplexFnDef::Implement> implements;
	//		{
	//			std::vector<ParserGenericParam> gparams;
	//			std::vector<ParserParameter>	params;
	//			{
	//				AstNodeType* another_value_type = new AstNodeType();
	//				another_value_type->InitWithIdentifier("str");
	//				params.push_back({ParserParameter{
	//					.name = "a",
	//					.type = another_value_type,
	//				}});
	//			}

	//			AstNodeType* return_type = new AstNodeType();
	//			return_type->InitWithIdentifier("str");

	//			implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
	//		}

	//		AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements, FnAttr::FN_ATTR_NONE);
	//		astnode_complex_fndef->Verify(ctx, VerifyContextParam());

	//		fns.push_back(astnode_complex_fndef);
	//	}

	//	AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
	//	TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_STR, TYPE_ID_STR});
	//	AddConstraint(constraint_tid, fns);
	//}
	// 手动实现Index约束
	//{
	//	std::vector<AstNodeComplexFnDef*> fns;
	//	{
	//		std::vector<AstNodeComplexFnDef::Implement> implements;
	//		{
	//			std::vector<ParserGenericParam> gparams;
	//			std::vector<ParserParameter>	params;
	//			{
	//				AstNodeType* index_type = new AstNodeType();
	//				index_type->InitWithIdentifier("i32");
	//				params.push_back({ParserParameter{
	//					.name = "a",
	//					.type = index_type,
	//				}});
	//			}

	//			AstNodeType* return_type = new AstNodeType();
	//			return_type->InitWithIdentifier("int");

	//			implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
	//		}

	//		AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("index", implements, FnAttr::FN_ATTR_NONE);
	//		astnode_complex_fndef->Verify(ctx, VerifyContextParam());

	//		fns.push_back(astnode_complex_fndef);
	//	}

	//	AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Index")->GetValueConstraint();
	//	TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_STR});
	//	AddConstraint(constraint_tid, fns);
	//}
	ctx.PopStack();
}
