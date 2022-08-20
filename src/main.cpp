#include <any>
#include <iostream>
#include <stdio.h>

#include "PinlangLexer.h"
#include "PinlangParser.h"
#include "antlr4-runtime.h"
#include "astnode_blockstmt.h"
#include "execute_context.h"
#include "verify_context.h"
#include "visitor.h"

#include "log.h"

using namespace antlr4;

int main(int argc, char* argv[]) {
	// const char *filename=argv[1];
	const char* filename = "../example_code/a.pin";
	log_info("filename[%s]", filename);

	std::ifstream infile(filename);
	ANTLRInputStream input;
	input.load(infile);
	PinlangLexer lexer(&input);
	CommonTokenStream tokens(&lexer);

	PinlangParser parser(&tokens);
	tree::ParseTree* tree = parser.start();
	//std::cout << tree->toStringTree(&parser, true) << std::endl << std::endl;

	Visitor visitor;
	AstNodeBlockStmt* block_stmt = std::any_cast<AstNodeBlockStmt*>(tree->accept(&visitor));


	VerifyContext vctx;
	vctx.PushStack();
	log_info("verify begin");
	block_stmt->Verify(vctx);
	log_info("verify end");

	ExecuteContext ectx;
	ectx.PushStack();
	log_info("execute begin");
	block_stmt->Execute(ectx);
	log_info("execute end");
	return 0;
}
