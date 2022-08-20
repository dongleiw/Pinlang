grammar Pinlang;
import Pinlang_lex;

type
    : TYPE
	| Identifier
	;

expr_primary
    : literal											# expr_primary_literal
    | Identifier										# expr_primary_identifier
    | L_PAREN expr R_PAREN								# expr_primary_parens
	| expr_primary L_PAREN expr_list R_PAREN			# expr_primary_fncall
	;

expr_list
	:
	| expr (',' expr)*
	;

expr
	: expr_primary																		# expr_primary_expr
    | expr op=(MUL|DIV|MOD) expr														# expr_muliplicative
    | expr op=(ADD|SUB) expr      														# expr_additive
    | expr op=(EQUALS|NOT_EQUALS|LESS|GREATER|LESS_OR_EQUALS|GREATER_OR_EQUALS) expr  	# expr_relational
    | expr op=(LOGICAL_OR| LOGICAL_AND) expr											# expr_logical
    ;

stmt_vardef
	: VAR Identifier type ';'
	| VAR Identifier type? ASSIGN expr ';'
	| CONST Identifier type? ASSIGN expr ';'
	;

stmt_block: L_CURLY statement* R_CURLY;

parameter: Identifier? type ;
parameter_list
	: 
	| parameter (',' parameter)*
	;
stmt_fndef
	: FN Identifier L_PAREN parameter_list R_PAREN type? stmt_block
	;

stmt_return
	: RETURN expr? ';'
	;

statement
	: expr ';'
	| stmt_vardef
	| stmt_fndef
	| stmt_block
	| stmt_return
    ;

literal
    :   FloatLiteral
    |   IntegerLiteral
    |	StringLiteral
    |	BoolLiteral
    ;

start: statement*;
