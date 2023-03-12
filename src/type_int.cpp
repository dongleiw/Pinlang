#include "type_int.h"

#include <assert.h>
#include <llvm-12/llvm/IR/Type.h>
#include <map>
#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_type.h"
#include "builtin_fn.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "log.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "variable_table.h"
#include "verify_context.h"

TypeInfoInt::TypeInfoInt(TypeId tid) {
	switch (tid) {
	case TYPE_ID_INT8:
		m_name			 = "i8";
		m_mem_size		 = 1;
		m_mem_align_size = 1;
		break;
	case TYPE_ID_INT16:
		m_name			 = "i16";
		m_mem_size		 = 2;
		m_mem_align_size = 2;
		break;
	case TYPE_ID_INT32:
		m_name			 = "i32";
		m_mem_size		 = 4;
		m_mem_align_size = 4;
		break;
	case TYPE_ID_INT64:
		m_name			 = "i64";
		m_mem_size		 = 8;
		m_mem_align_size = 8;
		break;
	case TYPE_ID_UINT8:
		m_name			 = "u8";
		m_mem_size		 = 1;
		m_mem_align_size = 1;
		break;
	case TYPE_ID_UINT16:
		m_name			 = "u16";
		m_mem_size		 = 2;
		m_mem_align_size = 2;
		break;
	case TYPE_ID_UINT32:
		m_name			 = "u32";
		m_mem_size		 = 4;
		m_mem_align_size = 4;
		break;
	case TYPE_ID_UINT64:
		m_name			 = "u64";
		m_mem_size		 = 8;
		m_mem_align_size = 8;
		break;
	default:
		panicf("unexpected type[%d:%s]", tid, GET_TYPENAME_C(tid));
		break;
	}
	m_typegroup_id	= TYPE_GROUP_ID_PRIMARY;
	m_is_value_type = true;
}
void TypeInfoInt::InitBuiltinMethods(VerifyContext& ctx) {
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现sub约束
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("sub", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Sub")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现mul约束
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("mul", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Mul")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现div约束
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("div", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Div")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid});
		AddConstraint(constraint_tid, fns);
	}
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
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("equal", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Equal")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现Less约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("lessThan", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("LessThan")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现LessEqual约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("lessEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("LessEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
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
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("notEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("NotEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
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
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterThan", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("GreaterThan")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	// 手动实现GreaterEqual约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* another_value_type = new AstNodeType();
					another_value_type->InitWithIdentifier(m_name);
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, BuiltinFn::compile_nop));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterEqual", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("GreaterEqual")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid});
		AddConstraint(constraint_tid, fns);
	}
	ctx.PopSTack();
}
llvm::Type* TypeInfoInt::GetLLVMIRType(CompileContext& cctx) {
	switch (m_typeid) {
	case TYPE_ID_INT8:
	case TYPE_ID_UINT8:
		return llvm::Type::getInt8Ty(IRC);
		break;
	case TYPE_ID_INT16:
	case TYPE_ID_UINT16:
		return llvm::Type::getInt16Ty(IRC);
		break;
	case TYPE_ID_INT32:
	case TYPE_ID_UINT32:
		return llvm::Type::getInt32Ty(IRC);
		break;
	case TYPE_ID_INT64:
	case TYPE_ID_UINT64:
		return llvm::Type::getInt64Ty(IRC);
		break;
	default:
		panicf("unexpected type[%d:%s]", m_typeid, GET_TYPENAME_C(m_typeid));
		break;
	}
}
