#include "predefine.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "fntable.h"
#include "execute_context.h"
#include "log.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"

/*
 * 一个简陋的格式化字符串函数
 * 只支持"{}"
 */
std::string format_string(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, std::vector<Variable*> args) {
	assert(builtin_fn_info.fn_list.size() + 1 == args.size());
	std::string value;
	std::string fmt			= args.at(0)->GetValueStr();
	bool		need_foramt = false;

	int arg_count = 1;
	for (size_t i = 0; i < fmt.size(); i++) {
		char ch = fmt.at(i);
		if (ch == '{') {
			need_foramt = true;
		} else if (need_foramt && ch == '}') {
			need_foramt	  = false;
			Variable* arg = args.at(arg_count);

			FnAddr	  fn_addr = builtin_fn_info.fn_list.at(arg_count - 1);
			Variable* str_v	   = ctx.GetFnTable().CallFn(fn_addr, ctx, arg, std::vector<Variable*>());

			value += str_v->GetValueStr();

			arg_count++;
		} else {
			value += ch;
		}
	}

	return value;
}
Variable* builtin_fn_printf(BuiltinFnInfo& builtin_fn_info, ExecuteContext& ctx, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr);
	assert(args.size() >= 1 && args.at(0)->GetTypeId() == TYPE_ID_STR && args.size() == builtin_fn_info.fn_list.size() + 1);

	std::string value = format_string(builtin_fn_info, ctx, args);

	printf(value.c_str());
	return nullptr;
}
static void builtin_fn_printf_verify(BuiltinFnInfo& builtin_fn, VerifyContext& ctx) {
	assert(builtin_fn.obj_tid == TYPE_ID_NONE);
	TypeInfoFn* ti_fn = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(builtin_fn.fn_tid));
	for (size_t i = 1; i < ti_fn->GetParamNum(); i++) {
		TypeId					 arg_tid		   = ti_fn->GetParamType(i);
		TypeInfo*				 ti_arg			   = g_typemgr.GetTypeInfo(arg_tid);
		std::vector<MethodIndex> method_index_list = ti_arg->GetConstraintMethod(ctx, "ToString", "tostring", std::vector<TypeId>());
		if (method_index_list.empty()) {
			panicf("type[%d:%s] not implement constraint ToString", builtin_fn.obj_tid, GET_TYPENAME_C(builtin_fn.obj_tid));
		} else if (method_index_list.size() > 1) {
			panicf("type[%d:%s] implement multiple constraint ToString", builtin_fn.obj_tid, GET_TYPENAME_C(builtin_fn.obj_tid));
		}
		FnAddr fn_addr = ti_arg->GetMethodByIdx(method_index_list.at(0));
		builtin_fn.fn_list.push_back(fn_addr);
	}
}
static AstNodeComplexFnDef::Implement create_implement_of_printf(int args_num) {

	AstNodeType* param_fmt_type = new AstNodeType();
	param_fmt_type->InitWithIdentifier("str");

	std::vector<ParserGenericParam> gparams;
	std::vector<ParserParameter>	params{ParserParameter{
		   .name = "fmt",
		   .type = param_fmt_type,
	   }};

	for (int i = 0; i < args_num; i++) {
		const std::string idx = int_to_str(i);
		gparams.push_back({ParserGenericParam{
			.type_name		 = "T" + idx,
			.constraint_name = "ToString",
		}});

		AstNodeType* value_type = new AstNodeType();
		value_type->InitWithIdentifier("T" + idx);
		params.push_back({ParserParameter{
			.name = "a" + idx,
			.type = value_type,
		}});
	}
	return AstNodeComplexFnDef::Implement(gparams, params, nullptr, builtin_fn_printf_verify, builtin_fn_printf);
}
static AstNodeComplexFnDef* create_astnode_printf() {
	std::vector<AstNodeComplexFnDef::Implement> implements;

	for (int i = 0; i < 16; i++) {
		implements.push_back(create_implement_of_printf(i));
	}
	AstNodeComplexFnDef* astnode_complex_fndef = new AstNodeComplexFnDef("printf", implements);

	return astnode_complex_fndef;
}

void register_predefine(AstNodeBlockStmt& astnode_block_stmt) {
	astnode_block_stmt.AddChildStmt(create_astnode_printf());
}
