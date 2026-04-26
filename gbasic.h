#include <complex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <errno.h>
#include <stdint.h>
#include <ctype.h>

#define MAX_TOKEN 25600
#define BFSIZE 256

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
#define IS_OP(c)                                                                 \
	((c) == '+' || (c) == '-' || (c) == '*' || (c) == '/')

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

typedef enum { //the order of which reflects the precedence.
	TOKEN_TYPE_NULL = 0,
    STRING_TOKEN,
    KEYWORD_TOKEN,
    RELOP_TOKEN,
    OP_TOKEN,
    FLOAT_TOKEN,
    INT_TOKEN,
	PAREN_TOKEN,
    IDENT_TOKEN,
	COMMA_TOKEN,
	SEMICOLON_TOKEN,
    SPACE_TOKEN,
    TOKEN_TYPE_END
} TokenType;

typedef enum {
	NODE_TYPE_NULL = 0,
	ROOT,
	EXPR,
	OR_EXPR,
	AND_EXPR,
	ADD_EXPR,
	MUL_EXPR,
	UNARY,
	LINENUM,
	INTEGER,
	FLOAT,
	INTEGERIDENT,
	FLOATIDENT,
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
} NodeType;

typedef union {
	double floatValue;
	int intValue;
	char string[BFSIZE];
	int linenum;
} Literal;

typedef struct {
	TokenType type;
	char lexeme[BFSIZE];
	Literal literal;
	int lineNum;
	int colNum;
} Token;

typedef struct ParseTreeNode {
	Token *token;
	NodeType type;
	unsigned int childCount;
	struct ParseTreeNode *children[32];
} ParseTreeNode;

typedef struct {
	int lineCount;
	ParseTreeNode **lines;
} Program;

typedef struct {
	Token *tokens;
	int tokenLen;
	Program *prog;
	// char token[64];
	Token *tokenPtr;
} ParserContext;

int lexer(char *bf, Token *tokens, int lineNum);
void parse(ParserContext *context);

// advance the token pointer by one after successfully parsing a token
ParseTreeNode *parseLine(ParserContext *context);
ParseTreeNode *parseLinenum(ParserContext *context);
ParseTreeNode *parseIntegerLiteral(ParserContext *context);
ParseTreeNode *parseFloatLiteral(ParserContext *context);
ParseTreeNode *parseStringLiteral(ParserContext *context);
ParseTreeNode *parseDigit(ParserContext *context);
ParseTreeNode *parseStatement(ParserContext *context);
ParseTreeNode *parseLetStatement(ParserContext *context);
ParseTreeNode *parseExpr(ParserContext *context);
ParseTreeNode *parseIfStatement(ParserContext *context);
ParseTreeNode *parsePrintStatement(ParserContext *context);
ParseTreeNode *parseInputStatement(ParserContext *context);
ParseTreeNode *parseIdentifier(ParserContext *context);
ParseTreeNode *parseEqual(ParserContext *context);
ParseTreeNode *parseTerm(ParserContext *context);
void *parseEOL(ParserContext *context);
ParseTreeNode *parseOrExpr(ParserContext *context);
ParseTreeNode *parseOrOperand(ParserContext *context);
ParseTreeNode *parseAndExpr(ParserContext *context);
ParseTreeNode *parseAndOperand(ParserContext *context);
ParseTreeNode *parseAddExpr(ParserContext *context);
ParseTreeNode *parseAddOperand(ParserContext *context);
ParseTreeNode *parseMulExpr(ParserContext *context);
ParseTreeNode *parseUnary(ParserContext *context);
ParseTreeNode *parsePrimary(ParserContext *context);
ParseTreeNode *parseMulOperand(ParserContext *context);
ParseTreeNode *parseRelOperator(ParserContext *context);
ParseTreeNode *parseUnaryOperand(ParserContext *context);
