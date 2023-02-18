#include "type_type.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "type_mgr.h"
#include "verify_context.h"

static Variable* builtin_fn_equal_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_TYPE && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_TYPE);
	return new Variable(thisobj->GetValueTid() == args.at(0)->GetValueTid());
}
static void builtin_fn_equal_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_TYPE);
}

static Variable* builtin_fn_tostring_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_TYPE && args.size() == 0);
	return new Variable(GET_TYPENAME(thisobj->GetValueTid()));
}
static void builtin_fn_tostring_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
}

static Variable* builtin_fn_getTypeName_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_TYPE && args.size() == 0);
	return new Variable(GET_TYPENAME(thisobj->GetValueTid()));
}
static void builtin_fn_getTypeName_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_TYPE);
}

static Variable* builtin_fn_getTypeId_execute(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_TYPE && args.size() == 0);
	return new Variable(int(thisobj->GetValueTid()));
}
static void builtin_fn_getTypeId_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	assert(builtin_fn_info.obj_tid == TYPE_ID_TYPE);
}

TypeInfoType::TypeInfoType() {
	m_name = "type";
	m_is_value_type	 = true;
}
void TypeInfoType::InitBuiltinMethods(VerifyContext& ctx) {
	ctx.PushStack();
	ctx.GetCurStack()->EnterBlock(new VariableTable());
	// 手动实现ToString约束
	{
		std::vector<AstNodeComplexFnDef*> fns;
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				AstNodeType*					return_type = new AstNodeType();
				return_type->InitWithIdentifier("str");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_tostring_verify));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("tostring", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		AddConstraint(constraint_tid, fns);

		GetConstraintMethod(ctx, "ToString", "tostring", std::vector<TypeId>()); // 触发tostring函数的实例化
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
					another_value_type->InitWithIdentifier("type");
					params.push_back({ParserParameter{
						.name = "a",
						.type = another_value_type,
					}});
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithIdentifier("bool");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_equal_verify));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("equal", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}

		AstNodeConstraint* constraint	  = ctx.GetCurStack()->GetVariable("Equal")->GetValueConstraint();
		TypeId			   constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_TYPE});
		AddConstraint(constraint_tid, fns);
	}
	// 增加内置方法
	{
		std::vector<AstNodeComplexFnDef*> fns;
		// 增加GetTypeName()str
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				AstNodeType*					return_type = new AstNodeType();
				return_type->InitWithIdentifier("str");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_getTypeName_verify));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("GetTypeName", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}
		// 增加GetTypeId()int
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				AstNodeType*					return_type = new AstNodeType();
				return_type->InitWithIdentifier("int");
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_getTypeId_verify));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("GetTypeId", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}
		AddConstraint(CONSTRAINT_ID_NONE, fns);
	}
	ctx.PopSTack();
}
