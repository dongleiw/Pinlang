grammar Pinlang;
import Pinlang_lex;

type
    : TYPE
	| Identifier
	;

primary_expr
    : literal											# primary_expr_literal
    | Identifier										# primary_expr_identifier
    | LPAREN expr RPAREN								# primary_expr_parens
	;

expr
	: primary_expr																		# expr_primary_expr
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

statement
	: expr ';'
	| stmt_vardef
    ;

literal
    :   FloatLiteral
    |   IntegerLiteral
    |	StringLiteral
    |	BoolLiteral
    ;

start: statement*;
