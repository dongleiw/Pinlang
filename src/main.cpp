#include <any>
#include <bits/getopt_core.h>
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "PinlangLexer.h"
#include "PinlangParser.h"
#include "antlr4-runtime.h"
#include "astnode_blockstmt.h"
#include "astnode_complex_fndef.h"
#include "define.h"
#include "dynamic_loading.h"
#include "execute_context.h"
#include "function_obj.h"
#include "predefine.h"
#include "support/Declarations.h"
#include "type_fn.h"
#include "type_mgr.h"
#include "utils.h"
#include "variable.h"
#include "verify_context.h"
#include "visitor.h"

#include "log.h"

using namespace antlr4;

std::vector<std::string> list_src_files(std::string src_path) {
	std::vector<std::string> files;
	std::filesystem::path	 path(src_path);
	if (std::filesystem::is_regular_file(path)) {
		files.push_back(src_path);
	} else if (std::filesystem::is_directory(path)) {
		for (auto iter : std::filesystem::directory_iterator(path)) {
			if (iter.is_regular_file()) {
				std::filesystem::path p = iter.path();
				if (has_suffix(p.filename(), ".pin")) {
					files.push_back(p.relative_path());
				}
			}
		}
	} else {
		panicf("unexpected src_path[%s]", src_path.c_str());
	}
	return files;
}
void execute(std::string src_path, std::vector<std::string> str_args) {
	log_info("execute src_path[%s]", src_path.c_str());

	std::vector<std::string> src_files = list_src_files(src_path);

	g_typemgr.InitTypes();

	// 加载predefine
	AstNodeBlockStmt* predefine_block_stmts;
	{
		const std::string predefine_filename = "../example_code/predefine.pin";
		ANTLRFileStream	  input;
		input.loadFromFile(predefine_filename);
		std::string		  ss = input.getSourceName();
		PinlangLexer	  lexer(&input);
		CommonTokenStream tokens(&lexer);

		PinlangParser	 parser(&tokens);
		tree::ParseTree* tree = parser.start();

		Visitor visitor;
		predefine_block_stmts = std::any_cast<AstNodeBlockStmt*>(tree->accept(&visitor));
		register_predefine(*predefine_block_stmts);
		DynamicLoading::RegisterFn(*predefine_block_stmts);
	}

	Visitor			  visitor;
	AstNodeBlockStmt* global_block_stmt = new AstNodeBlockStmt();
	global_block_stmt->SetGlobalBlock(true);
	global_block_stmt->AddPreDefine(*predefine_block_stmts);
	for (auto src_file : src_files) {
		log_info("load src file[%s]", src_file.c_str());
		ANTLRFileStream input;
		input.loadFromFile(src_file);
		PinlangLexer	  lexer(&input);
		CommonTokenStream tokens(&lexer);

		PinlangParser	 parser(&tokens);
		tree::ParseTree* tree = parser.start();

		AstNodeBlockStmt* block_stmt = std::any_cast<AstNodeBlockStmt*>(tree->accept(&visitor));
		global_block_stmt->MergeAnother(*block_stmt);
	}

	FunctionObj main_fn;
	{
		VerifyContext vctx(global_block_stmt);
		vctx.PushStack();
		log_info("verify begin");
		global_block_stmt->Verify(vctx, VerifyContextParam());
		log_info("verify end");

		Variable* main_complex_fn = vctx.GetGlobalVt()->GetVariableOrNull("main");
		if (main_complex_fn == nullptr) {
			panicf("main function not found");
		}
		AstNodeComplexFnDef*		  astnode_complex_fn_def = main_complex_fn->GetValueComplexFn();
		AstNodeComplexFnDef::Instance instance				 = astnode_complex_fn_def->Instantiate_type(vctx, g_typemgr.GetMainFnTid());
		main_fn												 = instance.fnobj;
	}

	ExecuteContext ectx;
	ectx.PushStack();
	log_info("execute begin");
	global_block_stmt->Execute(ectx);

	// 构造main函数的参数
	Variable* main_fn_arg;
	{
		std::vector<Variable*> str_var_array;
		for (auto iter : str_args) {
			str_var_array.push_back(new Variable(std::string(iter)));
		}
		TypeInfoFn* main_fn_ti = dynamic_cast<TypeInfoFn*>(g_typemgr.GetTypeInfo(g_typemgr.GetMainFnTid()));

		main_fn_arg = new Variable(main_fn_ti->GetParamType(0), str_var_array);
	}

	main_fn.Call(ectx, std::vector<Variable*>{main_fn_arg});
	log_info("execute end");

	printf("execute end. succ\n");
}

int main(int argc, char* argv[]) {
	init_log("./run.log");

	std::string				 src_path = "../example_code/test/a.pin";
	std::vector<std::string> main_fn_args{std::string("run")};

	int opt;
	int main_fn_arg_idx = 1;
	while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
		case 'f':
			src_path = std::string(optarg);
			main_fn_arg_idx += 2;
			break;
		default:
			fprintf(stderr, "unknown opt[%c]", opt);
			exit(1);
			break;
		}
	}

	for (; main_fn_arg_idx < argc; main_fn_arg_idx++) {
		main_fn_args.push_back(std::string(argv[main_fn_arg_idx]));
	}

	execute(src_path, main_fn_args);
	return 0;
}
