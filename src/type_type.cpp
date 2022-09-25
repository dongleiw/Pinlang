#include "type_type.h"
#include "astnode_constraint.h"
#include "define.h"
#include "function.h"
#include "type_mgr.h"
#include "verify_context.h"

static Variable* builtin_fn_equal(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_TYPE && args.size() == 1 && args.at(0)->GetTypeId() == TYPE_ID_TYPE);
	return new Variable(thisobj->GetValueTid() == args.at(0)->GetValueTid());
}
static Variable* builtin_fn_tostring(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj->GetTypeId() == TYPE_ID_TYPE && args.size() == 0);
	return new Variable(GET_TYPENAME(thisobj->GetValueTid()));
}

TypeInfoType::TypeInfoType() {
	m_name = "type";
}
void TypeInfoType::InitBuiltinMethods(VerifyContext& ctx) {
	// 实现constraint Equal
	{
		AstNodeConstraint*				 constraint		= ctx.GetCurStack()->GetVariable("Equal")->GetValueConstraint();
		TypeId							 constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_TYPE});
		std::map<std::string, Function*> methods;

		TypeId	  tid	 = g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{TYPE_ID_TYPE}, TYPE_ID_BOOL);
		Function* f		 = new Function(tid, std::vector<ConcreteGParam>(), builtin_fn_equal);
		methods["equal"] = f;

		AddConstraint(constraint_tid, methods);
	}
	// 实现constraint ToString
	{
		AstNodeConstraint*				 constraint		= ctx.GetCurStack()->GetVariable("ToString")->GetValueConstraint();
		TypeId							 constraint_tid = constraint->Instantiate(ctx, std::vector<TypeId>{});
		std::map<std::string, Function*> methods;

		TypeId	  tid		= g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{}, TYPE_ID_STR);
		Function* f			= new Function(tid, std::vector<ConcreteGParam>(), builtin_fn_tostring);
		methods["tostring"] = f;

		AddConstraint(constraint_tid, methods);
	}
}
