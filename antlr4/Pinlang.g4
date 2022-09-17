grammar Pinlang;
import Pinlang_lex;

type
    : TYPE
	| Identifier
	| FN L_PAREN parameter_list R_PAREN type?
	;

type_list
	: type (',' type)*
	;

expr_primary
    : literal                                           # expr_primary_literal
    | Identifier                                        # expr_primary_identifier
    | L_PAREN expr R_PAREN                              # expr_primary_parens
	| expr_primary L_PAREN expr_list R_PAREN            # expr_primary_fncall
	| Identifier L_BRACKET type_list R_BRACKET          # expr_primary_gparam       // 泛参的实例化, 数组下标使用()
	| expr_primary '.' Identifier                       # expr_primary_access_attr  // 访问属性
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

generic_param_constraint
	: Identifier
	| Identifier L_BRACKET type (',' type)* R_BRACKET
	;
generic_param: Identifier generic_param_constraint;
stmt_generic_fndef
	: FN Identifier L_BRACKET generic_param (',' generic_param)* R_BRACKET L_PAREN parameter_list R_PAREN type? stmt_block
	;

stmt_return
	: RETURN expr? ';'
	;

stmt_fn_declare
	: FN Identifier L_PAREN parameter_list R_PAREN type? ';'
	;

identifier_list: Identifier (',' Identifier)* ;

///// constraint
constraint_generic_params: L_BRACKET identifier_list R_BRACKET ;
stmt_constraint_def
	: CONSTRAINT Identifier constraint_generic_params? L_CURLY stmt_fn_declare* R_CURLY
	;

stmt_if 
	: IF expr stmt_block (ELSE IF expr stmt_block)* (ELSE stmt_block)?
	;

statement
	: expr ';'
	| stmt_vardef
	| stmt_fndef
	| stmt_generic_fndef
	| stmt_block
	| stmt_return
	| stmt_constraint_def
	| stmt_if
    ;

literal
    :   FloatLiteral
    |   IntegerLiteral
    |	StringLiteral
    |	BoolLiteral
    ;

start: statement*;
