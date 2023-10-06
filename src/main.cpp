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
#include "builtin_fn.h"
#include "compile_context.h"
#include "define.h"
#include "execute_context.h"
#include "fntable.h"
#include "function_obj.h"
#include "instruction.h"
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
void compile(std::string src_path, std::vector<std::string> str_args) {
	log_info("execute src_path[%s]", src_path.c_str());

	std::vector<std::string> src_files = list_src_files(src_path);

	g_typemgr.InitTypes();

	std::vector<AstNodeBlockStmt*> global_block_stmts;

	// 加载predefine
	global_block_stmts.push_back(load_predefine());

	// parse all source files
	for (auto src_file : src_files) {
		Visitor visitor;
		log_info("load src file[%s]", src_file.c_str());
		ANTLRFileStream input;
		input.loadFromFile(src_file);
		PinlangLexer	  lexer(&input);
		CommonTokenStream tokens(&lexer);

		PinlangParser	 parser(&tokens);
		tree::ParseTree* tree = parser.start();

		AstNodeBlockStmt* global_block_stmt = std::any_cast<AstNodeBlockStmt*>(tree->accept(&visitor));
		global_block_stmt->SetGlobalBlock(true);
		global_block_stmt->SetSrcFilename(src_file);
		global_block_stmts.push_back(global_block_stmt);
	}

	VerifyContext vctx(global_block_stmts);
	vctx.Verify();

	// 实例化出main函数
	std::string main_fn_name;
	{
		Variable* main_complex_fn = vctx.GetGlobalVt()->GetVariableOrNull("main");
		if (main_complex_fn == nullptr) {
			panicf("main function not found");
		}
		AstNodeComplexFnDef* astnode_complex_fn_def = main_complex_fn->GetValueComplexFn();
		main_fn_name								= astnode_complex_fn_def->Instantiate_type(vctx, g_typemgr.GetMainFnTid());
	}

	log_info("compile begin");
	CompileContext cctx;
	cctx.Init();
	BuiltinFn::register_external_fn(cctx);
	vctx.GetFnTable().Compile(cctx);
	cctx.GetModule().print(llvm::outs(), nullptr);
	log_info("compile end");
}

int main(int argc, char* argv[]) {
	init_log("./run.log");

	////
	// VM vm;
	// vm.test1();
	// vm.PrintInstructions();
	// vm.Start();
	// return 0;
	////

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

	compile(src_path, main_fn_args);
	return 0;
}
