#include <complex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <errno.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_TOKEN 25600

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_ALPHA(c)                                                              \
	(((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || (c) == '_')
#define IS_SPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')
#define IS_CHAR(c) (IS_ALPHA(c) || IS_DIGIT(c))
#define IS_CUOT(c) ((c) == '"' || (c) == '\'')
#define IS_PAREN(c) ((c) == '(' || (c) == ')')
#define IS_SYMBOL(c)                                                             \
	((c) == '+' || (c) == '-' || (c) == '*' || (c) == '/' || (c) == '^' ||       \
	(c) == '=' || (c) == '<' || (c) == '>' || (c) == '&' || (c) == '|' ||        \
	(c) == '$' || (c) == '%' || (c) == '.' || (c) == ',' || (c) == ';' ||        \
	IS_CUOT(c) || IS_PAREN(c))
#define IS_EOL(c) ((c) == '\0')

typedef enum {
	START,
	IN_CHAR,
	IN_SPACE,
    IN_SYMBOL,
	IN_PAREN,
	IN_CUOT,
	IN_OPERATOR,
	IN_DIGITS,
	IN_IDENTIFIER,
	IN_INT,
	IN_FLOAT,
	IN_STRING,
	IN_COMMENT,
	IN_STMT_OR_IDENTIFIER,
	IN_LEADING_LINENUM,
	BEFORE_STMT_OR_IDENTIFIER,
    FINAL

} LexerState;

typedef enum {
	TOKEN_TYPE_NULL = 0,
	KEYWORD_TOKEN,
	INT_TOKEN,
	FLOAT_TOKEN,
	STRING_TOKEN,
	SPACE_TOKEN,
	TOKEN_TYPE_END
} TokenType;

typedef enum {
	TYPE_NULL = 0,
	TOK_EOF,
	ROOT,
	EXPR,
	NEWLINE,
	LINENUM,
	INTEGER,
	FLOAT,
	INTEGERVAR,
	FLOATVAR,
	STRING,
	IDENTI,
	REM,
	LET,
	IF,
	THEN,
	PRINT,
	INPUT,
	FOR,
	TO,
	STEP,
	NEXT,
	GOTO,
	GOSUB,
	RETURN,
	END,
	PLUS,
	MINUS,
	STAR,
	SLASH,
	CARET,
	EQUALS,
	LT,
	GT,
	LE,
	GE,
	NE,
	AND,
	OR,
	NOT,
	COMMA,
	SEMICOLON,
	COLON,
	LPAREN,
	RPAREN
} Type;

typedef union {
	double floatValue;
	int intValue;
	char *string;
	int linenum;
} Literal;

typedef struct {
	Type type;
	char lexeme[64];
	Literal literal;
	int lineNum;
	int colNum;
} Token;

typedef struct {
	Token token;
	int type;
	unsigned int childCount;
	struct ParseTreeNode *children[32];
} ParseTreeNode;

typedef struct {
	int lineCount;
	ParseTreeNode **ParseTreeNode;
} Program;

typedef struct {
	Token **tokens;
	Program *prog;
	// char token[64];
	Token *tokenPtr;
} ParserContext;

int lexer(const char *bf, Token *tokens, int lineNum);
void parse(ParserContext *context);