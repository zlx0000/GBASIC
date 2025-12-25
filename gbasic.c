#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
//#include <errno.h>
#include <stdint.h>

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_OPERATOR(c) ((c) == '+' || (c) == '-' || (c) == '*' || (c) == '/' || \
						(c) == '^' || (c) == '=' || (c) == '<' || (c) == '>' || \
						(c) == '&' || (c) == '|')

struct Token
{
	int type;
	char lexeme[64];
	union
	{
		double floatValue;
		int intValue;
		char *string;
		int linenum;
	} literal;
	int lineNum;
	int colNum;
};

struct ParseTreeNode
{
	struct Token token;
	int type;
	// int childCount;
	struct ParseTreeNode *children[32];
};

struct Program
{
	int lineCount;
	struct ParseTreeNode **ParseTreeNode;
};

enum ParseType
{
	TOK_EOF = 0,
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
};

struct OperandQueueNode
{
	struct ParseTreeNode *ParseTreeNode;
	struct OperandQueueNode *prev;
	struct OperandQueueNode *next;
};

struct OperandQueue
{
	struct OperandQueueNode *nodes[256];
	struct OperandQueueNode *head;
	struct OperandQueueNode *tail;
	int size;
};

void initOperandQueue(struct OperandQueue *queue)
{
	queue->head = NULL;
	queue->tail = NULL;
	queue->size = 0;
	for (int i = 0; i < 256; i++)
	{
		queue->nodes[i] = NULL;
	}
}

void operandEnqueue(struct OperandQueue *queue, struct ParseTreeNode *node)
{
	if (queue->size >= 256)
	{
		perror("Operand queue overflow.");
		exit(EXIT_FAILURE);
	}
	struct OperandQueueNode *newNode = queue->nodes[queue->size++];
	newNode->ParseTreeNode = node;
	newNode->prev = queue->tail;
	newNode->next = NULL;
	if (queue->tail)
	{
		queue->tail->next = newNode;
	}
	else
	{
		queue->head = newNode;
	}
	queue->tail = newNode;
}

struct ParseTreeNode *operandDequeue(struct OperandQueue *queue)
{
	if (queue->size <= 0)
	{
		perror("Operand queue underflow.");
		exit(EXIT_FAILURE);
	}
	struct OperandQueueNode *node = queue->head;
	queue->head = node->next;
	if (queue->head)
	{
		queue->head->prev = NULL;
	}
	else
	{
		queue->tail = NULL;
	}
	queue->size--;
	return node->ParseTreeNode;
}

struct OperatorStack
{
	struct ParseTreeNode *items[256];
	struct ParseTreeNode *top;
	int size;
};

void initOperatorStack(struct OperatorStack *stack)
{
	stack->top = NULL;
	stack->size = 0;
	for (int i = 0; i < 256; i++)
	{
		stack->items[i] = NULL;
	}
}

void operatorPush(struct OperatorStack *stack, struct ParseTreeNode *node)
{
	if (stack->size >= 256)
	{
		perror("Operator stack overflow.");
		exit(EXIT_FAILURE);
	}
	stack->items[stack->size++] = node;
	stack->top = node;
}

struct ParseTreeNode *operatorPop(struct OperatorStack *stack)
{
	if (stack->size <= 0)
	{
		perror("Operator stack underflow.");
		exit(EXIT_FAILURE);
	}
	struct ParseTreeNode *node = stack->items[--stack->size];
	stack->top = (stack->size > 0) ? stack->items[stack->size - 1] : NULL;
	return node;
}

struct ParseTreeNode *operatorPeek(struct OperatorStack *stack)
{
	if (stack->size <= 0)
	{
		perror("Operator stack is empty.");
		exit(EXIT_FAILURE);
	}
	return stack->top;
}

struct ParserContext
{
	struct Token *token;
	struct Program *prog;
	//char token[64];
	struct Token *tokenPtr;
	struct OperandQueue *operandQueue;
	struct OperatorStack *operatorStack;
};

enum LexerState
{
	START,
	IN_NUMBER,
	IN_CHAR,
};

void lexer(const char *bf, struct Token *token)
{
	if (bf == NULL)
	{
		fprintf(stderr, "Buffer is NULL.\n");
		exit(EXIT_FAILURE);
	}
	if (token == NULL)
	{
		fprintf(stderr, "Token is NULL.\n");
		exit(EXIT_FAILURE);
	}
	enum LexerState state = START;
	int tokenIndex = 0;
	int lineNum = 1;
	int colNum = 1;
	while (*bf != '\0')
	{
		switch (state)
		{
		case START:
			if (*bf > '0' && *bf < '9')
			{
				state = IN_NUMBER;
			}
			if(*bf > 'a' && *bf < 'z' || *bf > 'A' && *bf < 'Z')
			{
				state = IN_CHAR;
			}
			break;
		case IN_NUMBER:
			break;
		case IN_CHAR:
			if (*bf > 'z' || *bf < 'a' && *bf > 'Z' || *bf < 'A')
			{
				state = START;
			}
			break;
		
		}
	}
}

//advance the token pointer by one after successfully parsing a token
struct ParseTreeNode *parseLine(struct ParserContext *context);
struct ParseTreeNode *parseLinenum(struct ParserContext *context);
struct ParseTreeNode *parseIntegerLiteral(struct ParserContext *context);
struct ParseTreeNode *parseFloatLiteral(struct ParserContext *context);
struct ParseTreeNode *parseStringLiteral(struct ParserContext *context);
struct ParseTreeNode *parseDigit(struct ParserContext *context);
struct ParseTreeNode *parseStatement(struct ParserContext *context);
struct ParseTreeNode *parseLetStatement(struct ParserContext *context);
struct ParseTreeNode *parseExpr(struct ParserContext *context);
struct ParseTreeNode *parseIfStatement(struct ParserContext *context);
struct ParseTreeNode *parsePrintStatement(struct ParserContext *context);
struct ParseTreeNode *parseInputStatement(struct ParserContext *context);
struct ParseTreeNode *parseIdentifier(struct ParserContext *context);
struct ParseTreeNode *parseEqual(struct ParserContext *context);
struct ParseTreeNode *parseTerm(struct ParserContext *context);
void *parseEOL(struct ParserContext *context);
struct ParseTreeNode *parseOrExpr(struct ParserContext *context);
struct ParseTreeNode *parseOrOperand(struct ParserContext *context);
struct ParseTreeNode *parseAndExpr(struct ParserContext *context);
struct ParseTreeNode *parseAndOperand(struct ParserContext *context);
struct ParseTreeNode *parseAddExpr(struct ParserContext *context);
struct ParseTreeNode *parseMulExpr(struct ParserContext *context);
struct ParseTreeNode *parseAddOperand(struct ParserContext *context);
struct ParseTreeNode *parseMulExpr(struct ParserContext *context);
struct ParseTreeNode *parseUnary(struct ParserContext *context);
struct ParseTreeNode *parsePrimary(struct ParserContext *context);
struct ParseTreeNode *parseMulOperand(struct ParserContext *context);
struct ParseTreeNode *parseRelOperator(struct ParserContext *context);
struct ParseTreeNode *parseUnaryOperand(struct ParserContext *context);

void parse(struct ParserContext *context)
{
	if (context->token == NULL || context->prog == NULL)
	{
		fprintf(stderr, "Parser context is not initialized.\n");
		return;
	}
	int lineNum = 0;
	while (context->tokenPtr != NULL)
	{
		context->prog->ParseTreeNode[lineNum] = parseLine(context);
	}
}

struct ParseTreeNode *parseLine(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	node->type = ROOT;
	node->children[0] = parseLinenum(context);
	node->children[1] = parseStatement(context);
	parseEOL(context);
	return node;
}

struct ParseTreeNode *parseLinenum(struct ParserContext *context)
{
	struct ParseTreeNode *node = parseIntegerLiteral(context);
	node->type = LINENUM;
	return node;
}

struct ParseTreeNode *parseIntegerLiteral(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	node->type = INTEGER;
	node->token.type = INTEGER;
	node->token.literal.intValue = atoi(context->tokenPtr->lexeme);
	context->tokenPtr++;
	return node;
}

struct ParseTreeNode *parseStatement(struct ParserContext *context)
{
	struct ParseTreeNode *node;

	if (strcmp(context->tokenPtr->lexeme, "LET") == 0)
	{
		node = parseLetStatement(context);
	}
	else if (strcmp(context->token->lexeme, "IF") == 0)
	{
		node = parseIfStatement(context);
	}
	else if (strcmp(context->token->lexeme, "PRINT") == 0)
	{
		node = parsePrintStatement(context);
	}
	else if (strcmp(context->token->lexeme, "INPUT") == 0)
	{
		node = parseInputStatement(context);
	}
	else
	{
		perror("Unknown statement type.");
		return NULL;
	}

	return node;
}

struct ParseTreeNode *parseLetStatement(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	node->type = LET;
	strcpy(context->token->lexeme, node->token.lexeme);
	context->tokenPtr = context->token;
	context->tokenPtr++;
	node->children[0] = parseIdentifier(context);
	node->children[1] = parseEqual(context);
	node->children[2] = parseExpr(context);
	return node;
}

struct ParseTreeNode *parseIfStatement(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	node->type = IF;
	strcpy(context->token->lexeme, node->token.lexeme);
	context->tokenPtr = context->token;
	context->tokenPtr++;
	node->children[0] = parseExpr(context);
	node->children[1] = parseRelOperator(context);
	node->children[2] = parseExpr(context);
	if (strcmp(context->tokenPtr->lexeme, "THEN") != 0)
	{
		perror("Expected THEN in IF statement.");
		return NULL;
	}
	node->children[3] = parseLinenum(context);
	return node;
}

struct ParseTreeNode *parseExpr(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	node->token.type = EXPR;
	node->children[0] = parseOrExpr(context);
	return node;
}

struct ParseTreeNode *parseOrExpr(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	node->children[0] = parseAndExpr(context);
	node->type = OR;
	if (!strcmp(context->token->lexeme, "OR"))
	{
		struct ParseTreeNode *node2 = parseOrOperand(context);
		node->children[1] = node2;
		struct ParseTreeNode *node3 = parseAndExpr(context);
		node->children[2] = node3;
	}
	return node;
}

struct ParseTreeNode *parseAndExpr(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	node->children[0] = parseAddExpr(context);
	node->type = AND;
	if (!strcmp(context->token->lexeme, "AND"))
	{
		struct ParseTreeNode *node2 = parseAndOperand(context);
		node->children[1] = node2;
		struct ParseTreeNode *node3 = parseAddExpr(context);
		node->children[2] = node3;
	}
	return node;
}

struct ParseTreeNode *parseAddExpr(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode * )calloc(1, sizeof(struct ParseTreeNode));
	node->children[0] = parseMulExpr(context);
	if (!strcmp(context->token->lexeme, "+") || !strcmp(context->token->lexeme, "-"))
	{
		struct ParseTreeNode *node2 = parseAddOperand(context);
		node->children[1] = node2;
		struct ParseTreeNode *node3 = parseMulExpr(context);
		node->children[2] = node3;
	}
	return node;
}

struct ParseTreeNode *parseMulExpr(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	node->children[0] = parseUnary(context);
	if (!strcmp(context->token->lexeme, "*") || !strcmp(context->token->lexeme, "/"))
	{
		struct ParseTreeNode *node2 = parseMulOperand(context);
		node->children[1] = node2;
		struct ParseTreeNode *node3 = parseUnary(context);
		node->children[2] = node3;
	}
	else
	{
		node->children[1] = NULL;
		node->children[2] = NULL;
	}
	return node;
}

struct ParseTreeNode *parseUnary(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	if (!strcmp(context->token->lexeme, "+") || !strcmp(context->token->lexeme, "-") || !strcmp(context->token->lexeme, "NOT"))
	{
		struct ParseTreeNode *node2 = parseUnaryOperand(context);
		node->children[0] = node2;
		struct ParseTreeNode *node3 = parsePrimary(context);
		node->children[1] = node3;
	}
	else
	{
		node->children[0] = NULL;
		struct ParseTreeNode *node2 = parsePrimary(context);
		node->children[1] = node2;
	}
	return node;
}

struct ParseTreeNode *parsePrimary(struct ParserContext *context)
{
	struct ParseTreeNode *node = (struct ParseTreeNode *)calloc(1, sizeof(struct ParseTreeNode));
	if (context->token->type == INTEGER)
	{
		node->children[0] = parseIntegerLiteral(context);
	}
	else if (context->token->type == FLOAT)
	{
		node->children[0] = parseFloatLiteral(context);
	}
	else if (context->token->type == STRING)
	{
		node->children[0] = parseStringLiteral(context);
	}
	else if (context->token->type == IDENTI)
	{
		node->children[0] = parseIdentifier(context);
	}
	else if (strcmp(context->token->lexeme, "(") == 0)
	{
		struct ParseTreeNode *node2 = parseExpr(context);
		node->children[0] = node2;
		if (strcmp(context->token->lexeme, ")") != 0)
		{
			perror("Expected closing parenthesis.");
			return NULL;
		}
	}
	else
	{
		perror("Invalid primary expression.");
		return NULL;
	}
}

int main(int argc, char **argv)
{
	if (argc <= 1)
	{
		printf("Usage: gbasic ./file\n");
	}
	if (*argv[1])
	{
		FILE *fp = fopen(argv[1], "r");
		if (fp == NULL)
		{
			perror("Can't open file.");
			exit(EXIT_FAILURE);
		}

		fseek(fp, 0, SEEK_END);
		unsigned long size = ftell(fp);
		rewind(fp);

		char *bf = (char *)malloc(size + 1);
		if (bf == NULL)
		{
			perror("Can't load the file, memory allocation failed");
			fclose(fp);
			exit(EXIT_FAILURE);
		}

		size_t byteRead = fread(bf, 1, size, fp);
		if (byteRead != size)
		{
			perror("Error reading file.");
			free(bf);
			fclose(fp);
			exit(EXIT_FAILURE);
		}
		bf[size] = '\0';
		fclose(fp);
		struct Program *prog = (struct Program *)calloc(1, sizeof(struct Program));
		if (prog == NULL)
		{
			perror("Memory allocation failed.");
			free(bf);
			exit(EXIT_FAILURE);
		}
		prog->ParseTreeNode = (struct ParseTreeNode **)calloc(1, sizeof(struct ParseTreeNode *) * 16384);
		if (prog->ParseTreeNode == NULL)
		{
			perror("Memory allocation failed.");
			free(prog);
			free(bf);
			exit(EXIT_FAILURE);
		}

		struct Token *token = (struct Token *)calloc(1, sizeof(struct Token) * 25600);

		lexer(bf, token);

		struct ParserContext *context = (struct ParserContext *)calloc(1, sizeof(struct ParserContext));
		context->token = token;
		context->tokenPtr = token;
		context->prog = prog;
		context->operatorStack = (struct OperatorStack *)calloc(1, sizeof(struct OperatorStack));
		context->operandQueue = (struct OperandQueue *)calloc(1, sizeof(struct OperandQueue));
		context->tokenPtr = context->token;
		if (context->operatorStack == NULL || context->operandQueue == NULL)
		{
			perror("Memory allocation failed.");
			free(context->prog->ParseTreeNode);
			free(context->prog);
			free(context->operandQueue);
			free(context->operatorStack);
			free(bf);
			free(context);
			exit(EXIT_FAILURE);
		}
		initOperatorStack(context->operatorStack);
		initOperandQueue(context->operandQueue);
		parse(context);
	}
}
