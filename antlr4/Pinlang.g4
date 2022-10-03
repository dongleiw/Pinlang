grammar Pinlang;
import Pinlang_lex;

type_array: L_BRACKET R_BRACKET type ;
type
    : TYPE
	| Identifier
	| FN L_PAREN parameter_list R_PAREN type?
	| type_array
	;

type_list
	: type (',' type)*
	;



expr_init_element
    : expr_init_body
    | ( Identifier ':' )? expr
    ;
expr_init_body
    : L_CURLY expr_init_element (',' expr_init_element)* ','? R_CURLY
    | L_CURLY R_CURLY
    ;
expr_init : type expr_init_body ;

expr_primary
    : literal                                           # expr_primary_literal
    | Identifier                                        # expr_primary_identifier
    | L_PAREN expr R_PAREN                              # expr_primary_parens
	| expr_primary L_PAREN expr_list R_PAREN            # expr_primary_fncall
	//| Identifier L_BRACKET type_list R_BRACKET          # expr_primary_gparam       // 提供泛参将泛型函数实例化. 由于仅仅通过泛参无法实例化, 这个先搁置了
	| expr_primary L_BRACKET expr R_BRACKET             # expr_primary_access_array_element // 数组下标访问
	| expr_primary '.' Identifier                       # expr_primary_access_attr  // 访问属性
	| expr_init                                         # expr_primary_init   // 初始化
	;

expr
	: expr_primary																		# expr_primary_expr
    | expr op=(MUL|DIV|MOD) expr														# expr_muliplicative
    | expr op=(ADD|SUB) expr      														# expr_additive
    | expr op=(EQUALS|NOT_EQUALS|LESS|GREATER|LESS_OR_EQUALS|GREATER_OR_EQUALS) expr  	# expr_relational
    | expr op=(LOGICAL_OR| LOGICAL_AND) expr											# expr_logical
    ;

expr_list
	:
	| expr (',' expr)*
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
	: stmt_simple_fndef
	| stmt_generic_fndef
	| stmt_complex_fndef
	;

////// 简单函数定义
stmt_simple_fndef
	: FN Identifier L_PAREN parameter_list R_PAREN type? stmt_block
	;
generic_param_constraint
	: Identifier
	| Identifier L_BRACKET type (',' type)* R_BRACKET
	;
generic_param: Identifier generic_param_constraint;

////// 泛型函数定义
stmt_generic_fndef
	: FN Identifier L_BRACKET generic_param (',' generic_param)* R_BRACKET L_PAREN parameter_list R_PAREN type? stmt_block
	;

////// 复杂函数定义
stmt_complex_fndef_implement
	: (L_BRACKET generic_param (',' generic_param)* R_BRACKET)? L_PAREN parameter_list R_PAREN type? stmt_block
	;
stmt_complex_fndef
	: FN Identifier L_CURLY stmt_complex_fndef_implement+ R_CURLY
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

///// if statement
stmt_if 
	: IF expr stmt_block (ELSE IF expr stmt_block)* (ELSE stmt_block)?
	;

///// 类定义
stmt_class_def_impl_constraint
	: 'impl' CONSTRAINT Identifier (L_BRACKET identifier_list R_BRACKET)? L_CURLY stmt_simple_fndef* R_CURLY
	;
stmt_class_def
	: CLASS Identifier L_CURLY (Identifier type ';' | stmt_fndef | stmt_class_def | stmt_class_def_impl_constraint)* R_CURLY
	;

//// 赋值
stmt_assignment
	: expr ASSIGN expr ';'
	;

statement
	: expr ';'
	| stmt_vardef
	| stmt_fndef
	| stmt_block
	| stmt_return
	| stmt_constraint_def
	| stmt_if
	| stmt_class_def
	| stmt_assignment
    ;

literal
    :   FloatLiteral
    |   IntegerLiteral
    |	StringLiteral
    |	BoolLiteral
    ;

start: statement*;
