#include "predefine.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "execute_context.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"

/*
 * 一个简陋的格式化字符串函数
 * 只支持"{}"
 */
std::string format_string(ExecuteContext& ctx, std::vector<Variable*> args) {
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

			// 调用tostring方法来转换为str
			// TODO 目前得先获取MethodIndex然后调用
			TypeInfo*	ti			 = g_typemgr.GetTypeInfo(arg->GetTypeId());
			MethodIndex method_index = ti->GetMethodIdx("tostring[]()str");
			//MethodIndex method_index = ti->GetConcreteMethod(ctx, "tostring[]()");
			Variable* str_v = arg->CallMethod(ctx, method_index, std::vector<Variable*>());

			value += str_v->GetValueStr().c_str();

			arg_count++;
		} else {
			value += ch;
		}
	}

	return value;
}
Variable* builtin_fn_printf(ExecuteContext& ctx, Function* fn, Variable* thisobj, std::vector<Variable*> args) {
	assert(thisobj == nullptr);
	assert(args.size() >= 1 && args.at(0)->GetTypeId() == TYPE_ID_STR);

	std::string value = format_string(ctx, args);

	printf(value.c_str());
	return nullptr;
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
	return AstNodeComplexFnDef::Implement(gparams, params, nullptr, nullptr, builtin_fn_printf);
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
