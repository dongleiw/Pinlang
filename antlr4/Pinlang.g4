grammar Pinlang;
import Pinlang_lex;

type_integer: INT_U8 | INT_U16 | INT_U32 | INT_U64 | INT_I8 | INT_I16 | INT_I32 | INT_I64 ;
type_float: FLOAT_F32 | FLOAT_F64 ;
type_bool: BOOL ;
type_str: STR ;
type_array_static_size: L_BRACKET expr R_BRACKET type ;
type_array_dynamic_size: L_BRACKET R_BRACKET type ;
type_array: type_array_static_size | type_array_dynamic_size ;
type_tuple: L_PAREN type_list R_PAREN ;
type_fn: FN L_PAREN parameter_list R_PAREN type? ;

type_value
	: TYPE
	| type_integer
	| type_float
	| type_bool
	| type_array_static_size
	| type_fn
	;

type_reference
	: type_array_dynamic_size
	| type_str
	| type_tuple
	;

type_pointer
	: '*' type
	;

type
    : type_value
	| type_reference
	| type_pointer
	| Identifier
	;

type_list
	: type (',' type)*
	;

/////// 初始化. (数组, 类名)
expr_init_element
    : expr_init_body
    | ( Identifier ':' )? expr
    ;
expr_init_body
    : L_CURLY expr_init_element (',' expr_init_element)* ','? R_CURLY
    | L_CURLY R_CURLY
    ;
expr_init : (type_array | Identifier) expr_init_body ;

expr_primary
    : literal                                           # expr_primary_literal
    | Identifier                                        # expr_primary_identifier
    | L_PAREN expr_list R_PAREN                         # expr_primary_parens  // 如果有多个expr, 则是tuple
	| expr_primary L_PAREN expr_list_optional R_PAREN   # expr_primary_fncall
	// | Identifier '::<' type_list GREATER             # expr_primary_gparam       // 提供泛参将泛型函数实例化. 由于仅仅通过泛参无法实例化, 这个先搁置了
	| expr_primary L_BRACKET expr R_BRACKET             # expr_primary_access_array_element // 数组下标访问
	| expr_primary '.' Identifier                       # expr_primary_access_attr  // 访问属性
	| expr_init                                         # expr_primary_init   // 初始化
	| '&' expr_primary                                  # expr_dereference   // value -> address
	| '*' expr_primary                                  # expr_reference   // address -> value
	;

expr
	: expr_primary																		# expr_primary_expr
    | expr op=(MUL|DIV|MOD) expr														# expr_muliplicative
    | expr op=(ADD|SUB) expr      														# expr_additive
    | expr op=(EQUALS|NOT_EQUALS|LESS|GREATER|LESS_OR_EQUALS|GREATER_OR_EQUALS) expr  	# expr_relational
    | expr op=(LOGICAL_OR | LOGICAL_AND | '!') expr                                     # expr_logical
    ;

expr_list_optional
	:
	| expr (',' expr)*
	;
expr_list: expr (',' expr)* ;

stmt_vardef
	: VAR Identifier type
	| VAR Identifier type? ASSIGN expr
	| CONST Identifier type? ASSIGN expr
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

////// return
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

///// for statement
stmt_for_init
	: expr
	| stmt_vardef
	| stmt_assignment
	|
	;
stmt_for_cond
	: expr
	|
	;
stmt_for_loop
	: expr
	| stmt_assignment
	|
	;
stmt_for
	: FOR stmt_for_init ';' stmt_for_cond ';' stmt_for_loop stmt_block
	;

///// while statement
stmt_while
	: WHILE stmt_for_cond stmt_block                      
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
	: expr ASSIGN expr
	;

//// break
stmt_break
	: BREAK ';'
	;

stmt_continue
	: CONTINUE ';'
	;

statement
	: expr ';'
	| stmt_vardef ';'
	| stmt_fndef
	| stmt_block
	| stmt_return
	| stmt_constraint_def
	| stmt_if
	| stmt_for
	| stmt_while
	| stmt_class_def
	| stmt_assignment ';'
	| stmt_break 
	| stmt_continue
    ;

literal
    :   FloatLiteral
    |   IntegerLiteral
    |	StringLiteral
    |	BoolLiteral
	|   PointerLiteral
    ;

start: statement*;
