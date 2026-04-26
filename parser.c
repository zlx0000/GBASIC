#include "gbasic.h"
#define CONSUME_TOKEN context->tokenPtr++

void parse(ParserContext *context)
{
	if (context->tokenPtr == NULL || context->prog == NULL) {
		fprintf(stderr, "Parser context is not initialized.\n");
		return;
	}
	int lineNum = 0;
	while (context->tokenPtr != NULL) {
		context->prog->lines[lineNum] = parseLine(context);
		lineNum++;
	}
}

ParseTreeNode *parseLine(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;
	node->type = ROOT;
	node->children[0] = parseLinenum(context);
	node->childCount++;
	node->children[1] = parseStatement(context);
	node->childCount++;
	if (context->tokenPtr - context->tokens != context->tokenLen - 1) {
		fprintf(stderr, "Token stream does not end at %d,%d: `%s`,\n", context->tokenPtr->lineNum, context->tokenPtr->colNum, context->tokenPtr->lexeme);
		return NULL;
	}
	//parseEOL(context);
	return node;
}

ParseTreeNode *parseLinenum(ParserContext *context)
{
	ParseTreeNode *node = parseIntegerLiteral(context);
	node->childCount = 0;
	node->type = LINENUM;
	return node;
}

ParseTreeNode *parseIntegerLiteral(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	if (context->tokenPtr->type != INT_TOKEN) {
		fprintf(stderr, "Not a integer at %d,%d: `%s`,\n", context->tokenPtr->lineNum, context->tokenPtr->colNum, context->tokenPtr->lexeme);
		return NULL;
	}
	node->childCount = 0;
	node->type = INTEGER;
	node->token = context->tokenPtr;
	CONSUME_TOKEN;
	return node;
}

ParseTreeNode *parseFloatLiteral(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	if (context->tokenPtr->type != FLOAT_TOKEN) {
		fprintf(stderr, "Not a float at %d,%d: `%s`,\n", context->tokenPtr->lineNum, context->tokenPtr->colNum, context->tokenPtr->lexeme);
		return NULL;
	}
	node->childCount = 0;
	node->type = FLOAT;
	node->token = context->tokenPtr;
	CONSUME_TOKEN;
	return node;
}

ParseTreeNode *parseStatement(ParserContext *context)
{
	ParseTreeNode *node;
	if (context->tokenPtr->type == KEYWORD_TOKEN) {
		if (strcasecmp(context->tokenPtr->lexeme, "LET") == 0)
			node = parseLetStatement(context);
		else if (strcasecmp(context->tokenPtr->lexeme, "IF") == 0)
			node = parseIfStatement(context);
		else if (strcasecmp(context->tokenPtr->lexeme, "PRINT") == 0)
			node = parsePrintStatement(context);
		else if (strcasecmp(context->tokenPtr->lexeme, "INPUT") == 0)
			node = parseInputStatement(context);
		else {
			fprintf(stderr, "Unknown statement type at %d,%d: `%s`,\n", context->tokenPtr->lineNum, context->tokenPtr->colNum, context->tokenPtr->lexeme);
			return NULL;
		}
	} else {
		fprintf(stderr, "Not a statement type at %d,%d: `%s`,\n", context->tokenPtr->lineNum, context->tokenPtr->colNum, context->tokenPtr->lexeme);
		return NULL;
	}

	return node;
}

ParseTreeNode *parseLetStatement(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;
	node->type = LET;
	node->token = context->tokenPtr;
	CONSUME_TOKEN;
	node->children[0] = parseIdentifier(context);
	node->childCount++;
	node->children[1] = parseEqual(context);
	node->childCount++;
	node->children[2] = parseExpr(context);
	node->childCount++;
  	return node;
}

ParseTreeNode *parseIfStatement(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;
	node->type = IF;
	node->token = context->tokenPtr;
	//context->tokenPtr = *context->tokens;
	CONSUME_TOKEN;
	node->children[0] = parseExpr(context);
	node->childCount++;
	node->children[1] = parseRelOperator(context);
	node->childCount++;
	node->children[2] = parseExpr(context);
	node->childCount++;
	if (context->tokenPtr->type != KEYWORD_TOKEN ||
		strcasecmp(context->tokenPtr->lexeme, "THEN") != 0) {
		fprintf(stderr, "Expected THEN in IF statement.\n");
		return NULL;
	}
	CONSUME_TOKEN;
	node->children[3] = parseLinenum(context);
	node->childCount++;
	return node;
}

ParseTreeNode *parseExpr(ParserContext *context)
{
	ParseTreeNode *node;
	node = parseOrExpr(context);
	return node;
}

ParseTreeNode *parseOrExpr(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;
	node->type = OR_EXPR;
	node->children[0] = parseAndExpr(context);
	node->childCount++;
	if (!strcasecmp(context->tokenPtr->lexeme, "OR")) {
		struct ParseTreeNode *node2 = parseOrOperand(context);
		node->children[1] = node2;
		node->childCount++;
		struct ParseTreeNode *node3 = parseAndExpr(context);
		node->children[2] = node3;
		node->childCount++;
	}
	return node;
}

ParseTreeNode *parseAndExpr(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;
	node->type = AND;
	node->children[0] = parseAddExpr(context);
	if (!strcasecmp(context->tokenPtr->lexeme, "AND")) {
		struct ParseTreeNode *node2 = parseAndOperand(context);
		node->children[1] = node2;
		node->childCount++;
		struct ParseTreeNode *node3 = parseAddExpr(context);
		node->children[2] = node3;
		node->childCount++;
	}
	return node;
}

ParseTreeNode *parseAddExpr(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;

	node->children[0] = parseMulExpr(context);
	if (!strcmp(context->tokenPtr->lexeme, "+") ||
		!strcmp(context->tokenPtr->lexeme, "-")) {
		struct ParseTreeNode *node2 = parseAddOperand(context);
		node->children[1] = node2;
		node->childCount++;
		struct ParseTreeNode *node3 = parseMulExpr(context);
		node->children[2] = node3;
		node->childCount++;
	}
	return node;
}

ParseTreeNode *parseMulExpr(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;

	node->children[0] = parseUnary(context);
	if (!strcmp(context->tokenPtr->lexeme, "*") ||
		!strcmp(context->tokenPtr->lexeme, "/")) {
		struct ParseTreeNode *node2 = parseMulOperand(context);
		node->children[1] = node2;
		node->childCount++;
		struct ParseTreeNode *node3 = parseUnary(context);
		node->children[2] = node3;
		node->childCount++;
	} else {
		node->children[1] = NULL;
		node->children[2] = NULL;
	}
	return node;
}

ParseTreeNode *parseUnary(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;
	if (!strcmp(context->tokenPtr->lexeme, "+") ||
		!strcmp(context->tokenPtr->lexeme, "-") ||
		!strcmp(context->tokenPtr->lexeme, "NOT")) {
		CONSUME_TOKEN;
		struct ParseTreeNode *node2 = parseUnaryOperand(context);
		node->children[0] = node2;
		node->childCount++;
		struct ParseTreeNode *node3 = parsePrimary(context);
		node->children[1] = node3;
		node->childCount++;
	} else {
		node->children[0] = NULL; // todo: add a placeholder for no unary operator
		node->childCount++;
		struct ParseTreeNode *node2 = parsePrimary(context);
		node->children[1] = node2;
		node->childCount++;
	}
	return node;
}

ParseTreeNode *parsePrimary(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;

	if (context->tokenPtr->type == INT_TOKEN) {
		node->children[0] = parseIntegerLiteral(context);
		node->childCount++;
	} else if (context->tokenPtr->type == FLOAT_TOKEN) {
		node->children[0] = parseFloatLiteral(context);
		node->childCount++;
	} else if (context->tokenPtr->type == STRING_TOKEN) {
		node->children[0] = parseStringLiteral(context);
		node->childCount++;
	} else if (context->tokenPtr->type == IDENT_TOKEN) {
		node->children[0] = parseIdentifier(context);
		node->childCount++;
	} else if (strcmp(context->tokenPtr->lexeme, "(") == 0){
		CONSUME_TOKEN;
		struct ParseTreeNode *node2 = parseExpr(context);
		node->children[0] = node2;
		node->childCount++;
		if (strcmp(context->tokenPtr->lexeme, ")") != 0) {
			CONSUME_TOKEN;
			fprintf(stderr, "Expected closing parenthesis.\n");
			return NULL;
		}
	} else {
		fprintf(stderr, "Invalid primary expression.\n");
		return NULL;
	}
	return node;
}
