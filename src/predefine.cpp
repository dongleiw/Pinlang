#include "predefine.h"
#include "astnode_blockstmt.h"
#include "visitor.h"

#include "ANTLRFileStream.h"
#include "CommonTokenStream.h"
#include "PinlangLexer.h"
#include "PinlangParser.h"
#include "PinlangVisitor.h"
#include "antlr4-runtime.h"

AstNodeBlockStmt* load_predefine() {
	AstNodeBlockStmt* predefine_block_stmts = nullptr;

	{
		const std::string predefine_filename = "../code/predefine/predefine.pin";

		antlr4::ANTLRFileStream input;
		input.loadFromFile(predefine_filename);
		std::string				  ss = input.getSourceName();
		PinlangLexer			  lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);

		PinlangParser			 parser(&tokens);
		antlr4::tree::ParseTree* tree = parser.start();

		Visitor visitor;
		predefine_block_stmts = std::any_cast<AstNodeBlockStmt*>(tree->accept(&visitor));
		predefine_block_stmts->SetGlobalBlock(true);
		predefine_block_stmts->SetSrcFilename(predefine_filename);
		BuiltinFn::register_builtin_fns(*predefine_block_stmts);
	}

	{
		std::vector<std::string> src_files = {
			"../code/predefine/u8.pin",
			"../code/predefine/u16.pin",
			"../code/predefine/u32.pin",
			"../code/predefine/u64.pin",
			"../code/predefine/i8.pin",
			"../code/predefine/i16.pin",
			"../code/predefine/i32.pin",
			"../code/predefine/i64.pin",
		};
		for (auto& srcfile : src_files) {
			antlr4::ANTLRFileStream input;
			input.loadFromFile(srcfile);
			std::string				  ss = input.getSourceName();
			PinlangLexer			  lexer(&input);
			antlr4::CommonTokenStream tokens(&lexer);

			PinlangParser			 parser(&tokens);
			antlr4::tree::ParseTree* tree = parser.start();

			Visitor			  visitor;
			AstNodeBlockStmt* block_stmt = std::any_cast<AstNodeBlockStmt*>(tree->accept(&visitor));
			predefine_block_stmts->MergeAnother(*block_stmt);
		}
	}

	return predefine_block_stmts;
}
