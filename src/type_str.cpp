#include "type_str.h"

#include <vector>

#include "define.h"
#include "execute_context.h"
#include "function.h"
#include "type.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "variable.h"
#include "astnode_restriction.h"

static Variable* builtin_fn_add_str(ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	std::string result = thisobj->GetValueStr() + args.at(0)->GetValueStr();
	return new Variable(result);
}
TypeInfoStr::TypeInfoStr() {
	m_name		   = "str";
	m_typegroup_id = TYPE_GROUP_ID_PRIMARY;
}
void TypeInfoStr::InitBuiltinMethods(VerifyContext& ctx) {
	{
		AstNodeRestriction*	   restriction	   = ctx.GetCurStack()->GetVariable("Add")->GetValueRestriction();
		TypeId				   restriction_tid = restriction->Instantiate(ctx, std::vector<TypeId>{TYPE_ID_STR, TYPE_ID_STR});
		std::map<std::string, Function*> methods;

		TypeId	  tid = g_typemgr.GetOrAddTypeFn(std::vector<TypeId>{TYPE_ID_STR}, TYPE_ID_STR);
		Function* f	  = new Function(tid, builtin_fn_add_str);
		methods["add"] = f;

		AddRestriction(restriction_tid, methods);
	}
}
