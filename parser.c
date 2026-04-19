#include "gbasic.h"

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


void parse(ParserContext *context)
{
	if (context->tokenPtr == NULL || context->prog == NULL) {
		fprintf(stderr, "Parser context is not initialized.\n");
		return;
	}
	int lineNum = 0;
	while (context->tokenPtr != NULL) {
		context->prog->ParseTreeNode[lineNum] = parseLine(context);
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
	parseEOL(context);
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
	node->childCount = 0;
	node->type = INTEGER;
	node->token.type = INTEGER;
	node->token.literal.intValue = atoi(context->tokenPtr->lexeme);
	context->tokenPtr++;
	return node;
}

ParseTreeNode *parseStatement(ParserContext *context)
{
	ParseTreeNode *node;

	if (strcmp(context->tokenPtr->lexeme, "LET") == 0)
		node = parseLetStatement(context);
	else if (strcmp(context->tokenPtr->lexeme, "IF") == 0)
		node = parseIfStatement(context);
	else if (strcmp(context->tokenPtr->lexeme, "PRINT") == 0)
		node = parsePrintStatement(context);
	else if (strcmp(context->tokenPtr->lexeme, "INPUT") == 0)
		node = parseInputStatement(context);
	else {
		fprintf(stderr, "Unknown statement type.\n");
		return NULL;
	}

	return node;
}

ParseTreeNode *parseLetStatement(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;node->type = LET;
	context->tokenPtr++;
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
	strcpy(context->tokenPtr->lexeme, node->token.lexeme);
	context->tokenPtr = *context->tokens;
	context->tokenPtr++;
	node->children[0] = parseExpr(context);
	node->childCount++;
	node->children[1] = parseRelOperator(context);
	node->childCount++;
	node->children[2] = parseExpr(context);
	node->childCount++;
	if (strcmp(context->tokenPtr->lexeme, "THEN") != 0) {
		fprintf(stderr, "Expected THEN in IF statement.\n");
		return NULL;
	}
	node->children[3] = parseLinenum(context);
	node->childCount++;
	return node;
}

ParseTreeNode *parseExpr(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;
	node->token.type = EXPR;
	node->children[0] = parseOrExpr(context);
	node->childCount++;
	return node;
}

ParseTreeNode *parseOrExpr(ParserContext *context)
{
	ParseTreeNode *node =
		(ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
	node->childCount = 0;
	node->type = OR;
	node->children[0] = parseAndExpr(context);
	node->childCount++;
	if (!strcmp(context->tokenPtr->lexeme, "OR")) {
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
	if (!strcmp(context->tokenPtr->lexeme, "AND")) {
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
		context->tokenPtr++;
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

	if (context->tokenPtr->type == INTEGER) {
		node->children[0] = parseIntegerLiteral(context);
		node->childCount++;
	} else if (context->tokenPtr->type == FLOAT) {
		node->children[0] = parseFloatLiteral(context);
		node->childCount++;
	} else if (context->tokenPtr->type == STRING) {
		node->children[0] = parseStringLiteral(context);
		node->childCount++;
	} else if (context->tokenPtr->type == IDENTI) {
		node->children[0] = parseIdentifier(context);
		node->childCount++;
	} else if (strcmp(context->tokenPtr->lexeme, "(") == 0){
		context->tokenPtr++;
		struct ParseTreeNode *node2 = parseExpr(context);
		node->children[0] = node2;
		node->childCount++;
		if (strcmp(context->tokenPtr->lexeme, ")") != 0) {
			context->tokenPtr++;
			fprintf(stderr, "Expected closing parenthesis.\n");
			return NULL;
		}
	} else {
		fprintf(stderr, "Invalid primary expression.\n");
		return NULL;
	}
	return node;
}
