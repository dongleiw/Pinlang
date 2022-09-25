#include <any>
#include <iostream>
#include <stdio.h>

#include "PinlangLexer.h"
#include "PinlangParser.h"
#include "antlr4-runtime.h"
#include "astnode_blockstmt.h"
#include "execute_context.h"
#include "type_mgr.h"
#include "verify_context.h"
#include "visitor.h"
#include "predefine.h"

#include "log.h"

using namespace antlr4;

int main(int argc, char* argv[]) {
	// const char *filename=argv[1];
	const char* filename = "../example_code/a.pin";
	if (argc >= 2) {
		filename = argv[1];
	}
	init_log("./run.log");
	log_info("filename[%s]", filename);

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

	std::ifstream	 infile(filename);
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
	return 0;
}
