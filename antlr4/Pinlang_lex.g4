lexer grammar Pinlang;

FloatLiteral
    :   Digit+ '.' Digit+ (FloatLiteralType)?
    |   Digit+ FloatLiteralType
    ;
IntegerLiteral
    : NonZeroDigit Digit* IntegerLiteralType?
    | '0' IntegerLiteralType?
    ;
StringLiteral
    : '"' (~["]  |  EscapeCharacter )* '"'
    // |  '"' EscapeCharacter+ '"'
    ;

BoolLiteral
    : TRUE
     |  FALSE
    ;

PointerLiteral
	: NULLPTR
	;

// keywords
FN			: 'fn';
VAR			: 'var';
TRUE		: 'true';
FALSE		: 'false';
NULLPTR		: 'null';
TYPE		: 'type';
RETURN		: 'return';
CLASS		: 'class';
PUBLIC		: 'public';
PRIVATE		: 'private';
PROTECTED	: 'protected';
STATIC		: 'static';
CONST       : 'const';
CONSTRAINT  : 'constraint';
IF          : 'if';
ELSE        : 'else';
FOR         : 'for';
WHILE       : 'while';
BREAK       : 'break';
CONTINUE    : 'continue';
INT_U8      : 'u8';
INT_U16     : 'u16';
INT_U32     : 'u32';
INT_U64     : 'u64';
INT_I8      : 'i8';
INT_I16     : 'i16';
INT_I32     : 'i32';
INT_I64     : 'i64';
FLOAT_F32   : 'f32';
FLOAT_F64   : 'f64';
BOOL        : 'bool';
NEW         : 'new';
CONSTRUCTOR : 'constructor';
AS          : 'as';

// Relation operators
EQUALS                 : '==';
NOT_EQUALS             : '!=';
LESS                   : '<';
LESS_OR_EQUALS         : '<=';
GREATER                : '>';
GREATER_OR_EQUALS      : '>=';

// Logical
LOGICAL_OR             : ' |  | ';
LOGICAL_AND            : '&&';

// operators
ADD				: '+';
SUB 			: '-';
MUL 			: '*';
DIV 			: '/';
MOD 			: '%';
L_PAREN			: '(';
R_PAREN 	    : ')';
L_CURLY         : '{';
R_CURLY         : '}';
L_BRACKET		: '[';
R_BRACKET		: ']';

// unused character ^ ' ~ $ @ # ` : 

ASSIGN : '=';

Identifier
    :   Letter (Letter  |  Digit)*
    ;


fragment Digit: [0-9];
fragment NonZeroDigit: [1-9];
fragment Letter :   [a-zA-Z_];
fragment IntegerLiteralType : 'i8' | 'i16' | 'i32' | 'i64' | 'u8' | 'u16' | 'u32' | 'u64' ;
fragment FloatLiteralType : 'f32' | 'f64' ;

/*
    \a   (Unicode value 0x07) alert or bell
    \b   (Unicode value 0x08) backspace
    \f   (Unicode value 0x0C) form feed
    \n   (Unicode value 0x0A) line feed or newline
    \r   (Unicode value 0x0D) carriage return
    \t   (Unicode value 0x09) horizontal tab
    \v   (Unicode value 0x0b) vertical tab
    \\   (Unicode value 0x5c) backslash
    \'   (Unicode value 0x27) single quote
*/
fragment EscapeCharacter : '\\' [abfnrtv\\"] ;

COMMENT :   '/*' .*? '*/' -> skip ;
LINE_COMMENT :   '//' ~[\r\n]* -> skip ;
WS:   [ \r\n\t]+ -> skip ;
