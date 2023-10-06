#include "type_int.h"

#include <assert.h>
#include <llvm-12/llvm/IR/Type.h>
#include <map>
#include <vector>

#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "astnode_dereference.h"
#include "astnode_fncall.h"
#include "astnode_identifier.h"
#include "astnode_return.h"
#include "astnode_type.h"
#include "builtin_fn.h"
#include "compile_context.h"
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
		m_original_name = "i8";
		break;
	case TYPE_ID_INT16:
		m_original_name = "i16";
		break;
	case TYPE_ID_INT32:
		m_original_name = "i32";
		break;
	case TYPE_ID_INT64:
		m_original_name = "i64";
		break;
	case TYPE_ID_UINT8:
		m_original_name = "u8";
		break;
	case TYPE_ID_UINT16:
		m_original_name = "u16";
		break;
	case TYPE_ID_UINT32:
		m_original_name = "u32";
		break;
	case TYPE_ID_UINT64:
		m_original_name = "u64";
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("add", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("Add")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("sub", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("Sub")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("mul", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("Mul")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("div", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("Div")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	// 手动实现mod约束
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("mod", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("Mod")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid, m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
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

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("Equal")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("lessThan", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("LessThan")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("lessEqual", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("LessEqual")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
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

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("NotEqual")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
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

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("GreaterThan")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
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
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("greaterEqual", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("GreaterEqual")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{m_typeid}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	// ToString约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					AstNodeType* self_type = new AstNodeType();
					self_type->InitWithIdentifier("self");

					AstNodeType* ptr_self_type = new AstNodeType();
					ptr_self_type->InitWithPointer(self_type);
					params.push_back(ParserParameter{
						.name = "",
						.type = ptr_self_type,
					});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("String");

				std::vector<AstNode*> args;
				AstNodeDereference*	  astnode_deref = new AstNodeDereference(new AstNodeIdentifier("self"));
				args.push_back(astnode_deref);
				std::string fnname;
				switch (m_typeid) {
				case TYPE_ID_INT8:
					fnname = "__compile_i8_tostring";
					break;
				case TYPE_ID_INT16:
					fnname = "__compile_i16_tostring";
					break;
				case TYPE_ID_INT32:
					fnname = "__compile_i32_tostring";
					break;
				case TYPE_ID_INT64:
					fnname = "__compile_i64_tostring";
					break;

				case TYPE_ID_UINT8:
					fnname = "__compile_u8_tostring";
					break;
				case TYPE_ID_UINT16:
					fnname = "__compile_u16_tostring";
					break;
				case TYPE_ID_UINT32:
					fnname = "__compile_u32_tostring";
					break;
				case TYPE_ID_UINT64:
					fnname = "__compile_u64_tostring";
					break;

				default:
					panicf("unexpected type[%d:%s]", m_typeid, GET_TYPENAME_C(m_typeid));
				}
				AstNodeFnCall* astnode_fncall = new AstNodeFnCall(new AstNodeIdentifier(fnname), args);

				AstNodeReturn* astnode_return = new AstNodeReturn(astnode_fncall);

				std::vector<AstNode*> stmts;
				stmts.push_back(astnode_return);
				AstNodeBlockStmt* astnode_block_stmt = new AstNodeBlockStmt(stmts);

				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, astnode_block_stmt));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("tostring", implements, FnAttr::FN_ATTR_NONE);
			astnode_complex_fndef->SetObjTypeId(m_typeid);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint		   = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		ConstraintInstance constraint_instance = constraint->Instantiate(ctx, std::vector<TypeId>{}, m_typeid);
		AddConstraint(constraint_instance, fns);
	}
	ctx.PopStack();
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
void TypeInfoInt::ConstructDefault(CompileContext& cctx, llvm::Value* obj) {
	// integer 默认初始化为0
	switch (m_typeid) {
	case TYPE_ID_INT8:
	case TYPE_ID_UINT8:
		IRB.CreateStore(IRB.getInt8(0), obj);
		break;
	case TYPE_ID_INT16:
	case TYPE_ID_UINT16:
		IRB.CreateStore(IRB.getInt16(0), obj);
		break;
	case TYPE_ID_INT32:
	case TYPE_ID_UINT32:
		IRB.CreateStore(IRB.getInt32(0), obj);
		break;
	case TYPE_ID_INT64:
	case TYPE_ID_UINT64:
		IRB.CreateStore(IRB.getInt64(0), obj);
		break;
	default:
		panicf("unexpected type[%d:%s]", m_typeid, GET_TYPENAME_C(m_typeid));
		break;
	}
}
