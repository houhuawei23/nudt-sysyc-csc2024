grammar SysY;

/*===-------------------------------------------===*/
/* Lexer rules                                     */
/*===-------------------------------------------===*/

// fragments

// keywords
CONST: 'const';
INT: 'int';
FLOAT: 'float';
VOID: 'void';
IF: 'if';
ELSE: 'else';
WHILE: 'while';
BREAK: 'break';
CONTINUE: 'continue';
RETURN: 'return';

// operators
ASSIGN: '=';
ADD: '+';
SUB: '-';
MUL: '*';
DIV: '/';
MODULO: '%';
LT: '<';
GT: '>';
LE: '<=';
GE: '>=';
EQ: '==';
NE: '!=';
AND: '&&';
OR: '||';
NOT: '!';

// punctuations
LPAREN: '(';
RPAREN: ')';
LBRACKET: '[';
RBRACKET: ']';
LBRACE: '{';
RBRACE: '}';
COMMA: ',';
SEMICOLON: ';';

// identifier
fragment ALPHA: [a-zA-Z];
fragment ALPHANUM: [a-zA-Z0-9];
fragment NONDIGIT: [a-zA-Z_];
ID: NONDIGIT (ALPHANUM | '_')*;

// literals
fragment DecDigit: [0-9];
fragment OctDigit: [0-7];
fragment HexDigit: [0-9a-fA-F];
fragment OctPrefix: '0';
fragment HexPrefix: '0' [xX];
fragment NonZeroDecDigit: [1-9];
fragment Sign: [+-];
fragment DecFractional: DecDigit* '.' DecDigit+ | DecDigit+ '.';
fragment Exponent: [eE] Sign? DecDigit+;
fragment DecFloat: DecFractional Exponent? | DecDigit+ Exponent;
fragment HexFractional: HexDigit* '.' HexDigit+ | HexDigit+ '.';
fragment BinExponent: [pP] Sign? DecDigit+;
fragment HexFloat:
	HexPrefix HexFractional BinExponent
	| HexPrefix HexDigit+ BinExponent;

ILITERAL:
	NonZeroDecDigit DecDigit*
	| OctPrefix OctDigit*
	| HexPrefix HexDigit+;

FLITERAL: DecFloat | HexFloat;

// string
fragment ESC: '\\"' | '\\\\';
STRING: '"' (ESC | .)*? '"';

// white space and comments
WS: [ \t\r\n] -> skip;
LINECOMMENT: '//' .*? '\r'? '\n' -> skip;
BLOCKCOMMENT: '/*' .*? '*/' -> skip;

/*===-------------------------------------------===*/
/* Syntax rules                                    */
/*===-------------------------------------------===*/

compUnit: (decl | funcDecl | funcDef)+;

// constDecl and varDecl shares the same syntax structure, except that constDecl must have constant
// initial values. We combine these two syntax rules, and ensure the constraint above at the
// semantic check phase.
decl: CONST? btype varDef (COMMA varDef)* SEMICOLON;

btype: INT | FLOAT;

varDef: lValue (ASSIGN initValue)?;

initValue: exp | LBRACE (initValue (COMMA initValue)*)? RBRACE;

funcDecl: funcType ID LPAREN funcFParams? RPAREN SEMICOLON; 

funcDef: funcType ID LPAREN funcFParams? RPAREN blockStmt;

funcType: VOID | INT | FLOAT;

funcFParams: funcFParam (COMMA funcFParam)*;

funcFParam: btype ID (LBRACKET RBRACKET (LBRACKET exp RBRACKET)*)?;

blockStmt: LBRACE blockItem* RBRACE;

blockItem: decl | stmt;

stmt:
	assignStmt
	| expStmt
	| ifStmt
	| whileStmt
	| breakStmt
	| continueStmt
	| returnStmt
	| blockStmt
	| emptyStmt;

assignStmt: lValue ASSIGN exp SEMICOLON;

expStmt: exp SEMICOLON;

ifStmt: IF LPAREN exp RPAREN stmt (ELSE stmt)?;

whileStmt: WHILE LPAREN exp RPAREN stmt;

breakStmt: BREAK SEMICOLON;

continueStmt: CONTINUE SEMICOLON;

returnStmt: RETURN exp? SEMICOLON;

emptyStmt: SEMICOLON;

exp:
	LPAREN exp RPAREN				# parenExp
	| var								# varExp
	| number						# numberExp
	| string						# stringExp
	| call							# callExp
	| (ADD | SUB | NOT) exp			# unaryExp
	| exp (MUL | DIV | MODULO) exp	# multiplicativeExp
	| exp (ADD | SUB) exp			# additiveExp
	| exp (LT | GT | LE | GE) exp	# relationExp
	| exp (EQ | NE) exp				# equalExp
	| exp AND exp					# andExp
	| exp OR exp					# orExp;

call: ID LPAREN funcRParams? RPAREN;

var: ID (LBRACKET exp RBRACKET)*;

lValue: ID (LBRACKET exp RBRACKET)*;

number: ILITERAL | FLITERAL;

string: STRING;

funcRParams: exp (COMMA exp)*;