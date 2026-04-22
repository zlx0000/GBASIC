#include "gbasic.h"
#include "keywords.h"

Literal literal(TokenType t, char *lexeme);

typedef enum {
	MISMATCH,
	PENDING,
	MATCH
} DFA_state;

DFA_state is_int(char *t)
{
	char *p = t;
	enum state {
		start,
		//sign, //signs should be standalone tokens.
		digit
	} s;
	s = start;
	while (true) {
		switch (s) {
			case start:
				if (IS_DIGIT(*p))
					s = digit;
				//else if ((*p) == '+' || (*p) == '-')
				//	s = sign;
				else
					return MISMATCH;
				p++;
				break;
			/*	
			case sign:
				if (IS_DIGIT(*p))
					s = digit;
				else
					return MISMATCH;
				p++;
				break;
			*/
			case digit:
				if (IS_DIGIT(*p))
					s = digit;
				else if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
				p++;
				break;
		}
	}
}

DFA_state is_float(char *t)
{
	char *p = t;
	enum state {
		start,
		//sign, //signs should be standalone tokens.
		digit1,
		period,
		digit2
	} s;
	s = start;
	while (true) {
		switch (s) {
			case start:
				if (IS_DIGIT(*p))
					s = digit1;
				//else if ((*p) == '+' || (*p) == '-')
				//	s = sign;
				else
					return MISMATCH;
				p++;
				break;
			/*
			case sign:
				if (IS_DIGIT(*p))
					s = digit1;
				else if (IS_EOL(*p))
					return PENDING;
				else
					return MISMATCH;
				p++;
				break;
			*/
			case digit1:
				if (IS_DIGIT(*p))
					s = digit1;
				else if ((*p) == '.')
					s = period;
				else if (IS_EOL(*p))
					return PENDING;
				else
					return MISMATCH;
				p++;
				break;
			case period:
				if (IS_DIGIT(*p))
					s = digit2;
				else if (IS_EOL(*p))
					return PENDING;
				else
					return MISMATCH;
				p++;
				break;
			case digit2:
				if (IS_DIGIT(*p))
					s = digit2;
				else if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
				p++;
				break;
		}
	}
}

DFA_state is_space(char *t)
{
	char *p = t;
	enum state {
		a,
		b
	} s;
	s = a;
	while (true) {
		switch (s) {
			case a:
				if (IS_SPACE(*p))
					s = b;
				else
					return MISMATCH;
				p++;
				break;
			case b:
				if (IS_SPACE(*p))
					s = b;
				else if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
				p++;
				break;
		}
	}
}

DFA_state is_str(char *t)
{
	char *p = t;
	enum state {
		q1,
		str,
		escape,
		q2
	} s;
	s = q1;
	while (true) {
		switch (s) {
			case q1:
				if ((*p) == '"')
					s = str;
				else
					return MISMATCH;
				p++;
				break;
			case str:
				if ((*p) == '"')
					s = q2;
				else if ((*p) == '\\')
					s = escape;
				else if (IS_EOL(*p))
					return PENDING;
				else
					s = str;
				p++;
				break;
			case q2:
				if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
			case escape:
				if (IS_EOL(*p))
					return MISMATCH;
				p++;
				s = str;
				break;
		}
	}
}

DFA_state is_keyword(char* t)
{
    for (int i = 0; i < KEYWORDS_SIZE; i++) {
        size_t len_t = strlen(t);
        size_t len_k = strlen(keywords[i]);

        if (strncmp(t, keywords[i], len_t) == 0) {
            if (len_t == len_k)
                return MATCH;
            else
                return PENDING;
        }
    }
    return MISMATCH;
}

DFA_state is_ident(char *t)
{
	char *p = t;
	enum state {
		alpha,
		alpha_and_digits,
		suffix
	} s;
	s = alpha;
	while (true) {
		switch (s) {
			case alpha:
				if (IS_ALPHA(*p))
					s = alpha_and_digits;
				
				else
					return MISMATCH;
				p++;
				break;
			case alpha_and_digits:
				if (IS_CHAR(*p))
					s = alpha_and_digits;
				else if (*p == '$' || *p == '%')
					s = suffix;
				else if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
				p++;
				break;
			case suffix:
				if (IS_EOL(*p))
					return MATCH;
				else
					return MISMATCH;
				break;
		}
	}
}

DFA_state is_equal(char *t)
{
	int len = strlen(t);
	if (len > 2)
		return MISMATCH;
	if (len == 1) {
		if (t[0] == '=')
				return PENDING;
			else
				return MISMATCH;
	}
	if (len == 2) {
		if (t[0] == '=') {
			if (t[1] == '\0')
				return MATCH;
			else
				return MISMATCH;
		}
		else
			return MISMATCH;
	}
}

size_t lexer(char *bf, Token *tokens, int lineNum)
{
	char *bfend = bf;
	while (*bfend != '\0') bfend++;
	if (bfend == bf) return 0;
	DFA_state states[64];
	struct longest_match {
		char *end;
		size_t len;
		TokenType type;
	} m;
	m.end = bf;
	m.len = 0;
	m.type = TOKEN_TYPE_NULL;
	char *start = bf;
	char *end = bf;
	size_t tokenslen = 0;

	for (int i = 0; i < 64; i++)
    	states[i] = PENDING;
	char t[64];
	memset(t, 0, sizeof(t));
	size_t tlen = 0;

	while (possible(states) && end != '\0') {
		end++;
		tlen++;
		for (TokenType i = TOKEN_TYPE_NULL+1; i < TOKEN_TYPE_END; i++) {
			switch (i) {
				case INT_TOKEN:
					if (states[INT_TOKEN] != MISMATCH)
						states[INT_TOKEN] = is_int(t);
					break;
				case FLOAT_TOKEN:
					if (states[FLOAT_TOKEN] != MISMATCH)
						states[FLOAT_TOKEN] = is_float(t);
					break;
				/*		And so on ..	*/
			}
		}
		for (TokenType i = TOKEN_TYPE_NULL+1; i < TOKEN_TYPE_END; i++) {
			if (states[i] == MATCH) {
				if (tlen > m.len) {
					m.len = tlen;
					m.end = end;
					m.type = i;
				}
			}
		}
	}
	end--;
	tlen--;
	if (m.type != TOKEN_TYPE_NULL) {
		strncpy(tokens[tokenslen].lexeme, start, tlen);
		tokens[tokenslen].literal = literal(m.type, tokens[tokenslen].lexeme);
		tokens[tokenslen].type = m.type;
	}
	return tokenslen;
}