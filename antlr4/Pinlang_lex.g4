lexer grammar Pinlang;

FloatLiteral
    :   Digit+ '.' Digit+
    |   '.' Digit+
    |   Digit+ '.'
    ;
IntegerLiteral
    : NonZeroDigit Digit*
    | '0'
    ;
StringLiteral
    :   '"' ~["]* '"'
    |  '"' EscapeCharacter+ '"'
    ;

BoolLiteral
    : TRUE
    | FALSE
    ;

// keywords
FN			: 'fn';
VAR			: 'var';
TRUE		: 'true';
FALSE		: 'false';
TYPE		: 'type';
RETURN		: 'return';
CLASS		: 'class';
PUBLIC		: 'public';
PRIVATE		: 'private';
PROTECTED	: 'protected';
STATIC		: 'static';
CONST       : 'const';

// Relation operators
EQUALS                 : '==';
NOT_EQUALS             : '!=';
LESS                   : '<';
LESS_OR_EQUALS         : '<=';
GREATER                : '>';
GREATER_OR_EQUALS      : '>=';

// Logical
LOGICAL_OR             : 'or';
LOGICAL_AND            : 'and';

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

// unused character ^ ! ' ~ ^ $ @ # ` : |

ASSIGN : '=';

Identifier
    :   Letter (Letter | Digit)*
    ;


fragment Digit: [0-9];
fragment NonZeroDigit: [1-9];
fragment Letter :   [a-zA-Z_];
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
