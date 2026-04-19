#include "gbasic.h"

int lexer(const char *bf, Token *tokens, int lineNum)
{
	if (bf == NULL) {
		fprintf(stderr, "Buffer is NULL.\n");
		exit(EXIT_FAILURE);
	}
	if (tokens == NULL) {
		fprintf(stderr, "Token is NULL.\n");
		exit(EXIT_FAILURE);
	}
	LexerState state = START;
	int tokenIndex = 0;
	int colNum = 1;
	char lexeme[64];
	int lexemeLen = 0;
	while (true) {
		switch (state) {
			case START:
				if (IS_DIGIT(*bf))
					state = IN_CHAR;
				else {
				 	perror("Expecting line number at the beginning of the line.");
					return -1;
                }
				break;
			case IN_SPACE:
				*bf++;
				colNum++;
				if (IS_SPACE(*bf))
					state = IN_SPACE;
				else if (IS_SYMBOL(*bf))
					state = IN_SYMBOL;
				else if (IS_CHAR(*bf))
					state = IN_CHAR;
				else if (IS_PAREN(*bf))
					state = IN_PAREN;
                else if (IS_EOL(*bf))
                    state = FINAL;
				else {
					perror("invalid character");
					return -1;
                }
				break;
			case IN_CHAR:
				lexeme[lexemeLen] = *bf++;
				tokens[tokenIndex].lexeme[lexemeLen] = lexeme[lexemeLen];
                lexemeLen++;
				tokens[tokenIndex].lineNum = lineNum;
                if (!IS_CHAR(*bf)) {
					tokens[tokenIndex].colNum = colNum;
					colNum += lexemeLen;
                    tokenIndex++;
                    lexemeLen = 0;
                }
				if (IS_SPACE(*bf))
					state = IN_SPACE;
				else if (IS_SYMBOL(*bf))
					state = IN_SYMBOL;
				else if (IS_CHAR(*bf))
					state = IN_CHAR;
				else if (IS_PAREN(*bf))
					state = IN_PAREN;
                else if (IS_EOL(*bf))
                    state = FINAL;
				else {
					perror("invalid character");
					return -1;
                }
				break;
			case IN_SYMBOL:
				lexeme[lexemeLen] = *bf++;
				tokens[tokenIndex].lexeme[lexemeLen] = lexeme[lexemeLen];
                lexemeLen++;
				tokens[tokenIndex].lineNum = lineNum;
                if (!IS_SYMBOL(*bf)) {
					tokens[tokenIndex].colNum = colNum;
					colNum += lexemeLen;
                    tokenIndex++;
                    lexemeLen = 0;
                }
				if (IS_SPACE(*bf))
					state = IN_SPACE;
				else if (IS_SYMBOL(*bf))
					state = IN_SYMBOL;
				else if (IS_CHAR(*bf))
					state = IN_CHAR;
				else if (IS_PAREN(*bf))
					state = IN_PAREN;
                else if (IS_EOL(*bf))
                    state = FINAL;
				else {
					perror("invalid character");
					return -1;
                }
				break;
			case IN_PAREN:
				lexeme[lexemeLen] = *bf++;
				tokens[tokenIndex].colNum = colNum;
				tokens[tokenIndex].lexeme[lexemeLen] = lexeme[lexemeLen];
                lexemeLen++;
				tokens[tokenIndex].lineNum = lineNum;
				colNum++;
                tokenIndex++;
                lexemeLen = 0;
				if (IS_SPACE(*bf))
					state = IN_SPACE;
				else if (IS_SYMBOL(*bf))
					state = IN_SYMBOL;
				else if (IS_CHAR(*bf))
					state = IN_CHAR;
				else if (IS_PAREN(*bf))
					state = IN_PAREN;
                else if (IS_EOL(*bf))
                    state = FINAL;
				else {
					perror("invalid character");
					return -1;
                }
				break;
            case FINAL:
                return tokenIndex;
		}
	}
}