#include <any>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "PinlangLexer.h"
#include "PinlangParser.h"
#include "antlr4-runtime.h"
#include "astnode_blockstmt.h"
#include "execute_context.h"
#include "predefine.h"
#include "type_mgr.h"
#include "verify_context.h"
#include "visitor.h"

#include "log.h"

using namespace antlr4;

void execute(std::string filepath) {
	log_info("execute filename[%s]", filepath.c_str());

	g_typemgr.InitTypes();
	VariableTable::GetGlobal().InitAsGlobal();

	// 加载predefine
	AstNodeBlockStmt* predefine_block_stmts;
	{
		const std::string predefine_filename = "../example_code/predefine.pin";
		std::ifstream	  infile(predefine_filename);
		ANTLRInputStream  input;
		input.load(infile);
		PinlangLexer	  lexer(&input);
		CommonTokenStream tokens(&lexer);

		PinlangParser	 parser(&tokens);
		tree::ParseTree* tree = parser.start();
		//std::cout << tree->toStringTree(&parser, true) << std::endl << std::endl;

		Visitor visitor;
		predefine_block_stmts = std::any_cast<AstNodeBlockStmt*>(tree->accept(&visitor));
		register_predefine(*predefine_block_stmts);
	}

	std::ifstream	 infile(filepath);
	ANTLRInputStream input;
	input.load(infile);
	PinlangLexer	  lexer(&input);
	CommonTokenStream tokens(&lexer);

	PinlangParser	 parser(&tokens);
	tree::ParseTree* tree = parser.start();
	//std::cout << tree->toStringTree(&parser, true) << std::endl << std::endl;

	Visitor			  visitor;
	AstNodeBlockStmt* block_stmt = std::any_cast<AstNodeBlockStmt*>(tree->accept(&visitor));
	block_stmt->AddPreDefine(*predefine_block_stmts);

	{
		VerifyContext vctx;
		vctx.PushStack();
		log_info("verify begin");
		block_stmt->Verify(vctx, VerifyContextParam());
		log_info("verify end");
	}

	ExecuteContext ectx;
	ectx.PushStack();
	log_info("execute begin");
	block_stmt->Execute(ectx);
	log_info("execute end");

	printf("execute end. succ\n");
}

int main(int argc, char* argv[]) {
	const std::string default_filepath = "../example_code/a.pin";
	init_log("./run.log");

	std::string filepath;

	int opt;
	while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
		case 'f':
			filepath = std::string(optarg);
			break;
		default:
			fprintf(stderr, "unknown opt[%c]", opt);
			exit(1);
			break;
		}
	}

	if (filepath.empty()) {
		filepath = default_filepath;
	}
	execute(filepath);
	return 0;
}
