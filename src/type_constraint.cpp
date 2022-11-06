#include "type_constraint.h"
#include "astnode_complex_fndef.h"
#include "astnode_constraint.h"
#include "define.h"
#include "fntable.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "type_virtual_gtype.h"
#include "utils.h"
#include "verify_context.h"

static Variable* builtin_fn_never_call(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	panicf("bug");
	return nullptr;
}

static void builtin_fn_never_call_verify(BuiltinFnInfo& builtin_fn_info, VerifyContext& ctx) {
	TypeInfoArray* ti_array		= dynamic_cast<TypeInfoArray*>(g_typemgr.GetTypeInfo(builtin_fn_info.obj_tid));
	TypeInfo*	   ti_element	= g_typemgr.GetTypeInfo(ti_array->GetElementType());
	MethodIndex	   method_index = ti_element->GetMethodIdx("tostring[]()str");
	FnAddr fn_addr = ti_element->GetMethodByIdx(method_index);
	builtin_fn_info.fn_list.push_back(fn_addr);
}

void TypeInfoConstraint::FillVirtualType(VerifyContext& ctx, TypeInfoVirtualGType& ti) const {
	std::vector<AstNodeComplexFnDef*> fns;
	for (auto rule : m_rules) {
		TypeInfoFn* tifn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(rule.fn_tid));
		{
			std::vector<AstNodeComplexFnDef::Implement> implements;
			{
				std::vector<ParserGenericParam> gparams;
				std::vector<ParserParameter>	params;
				{
					for (size_t i = 0; i < tifn->GetParamNum(); i++) {
						AstNodeType* param_type = new AstNodeType();
						param_type->InitWithTargetTypeId(tifn->GetParamType(i));
						params.push_back(ParserParameter{
							.name = "a" + int_to_str(i),
							.type = param_type,
						});
					}
				}
				AstNodeType* return_type = new AstNodeType();
				return_type->InitWithTargetTypeId(tifn->GetReturnTypeId());
				implements.push_back(AstNodeComplexFnDef::Implement(gparams, params, return_type, builtin_fn_never_call_verify, builtin_fn_never_call));
			}
			AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("tostring", implements);
			astnode_complex_fndef->Verify(ctx, VerifyContextParam());
			fns.push_back(astnode_complex_fndef);
		}
	}

	ti.AddConstraint(GetTypeId(), fns);
}
