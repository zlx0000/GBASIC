#include "gbasic.h"

int main(int argc, char **argv)
{
	FILE *fp = NULL;
	if (argc <= 1)
		printf("Usage: gbasic ./file\n");
	if (*argv[1]) {
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			fprintf(stderr, "Can't open file.\n");
			exit(EXIT_FAILURE);
		}
	}

	fseek(fp, 0, SEEK_END);
	unsigned long size = ftell(fp);
	rewind(fp);

	char *bf = (char *)malloc(size + 1);
	if (bf == NULL) {
		fprintf(stderr, "Can't load the file, memory allocation failed\n");
		fclose(fp);
		exit(EXIT_FAILURE);
	}

	size_t byteRead = fread(bf, 1, size, fp);
	if (byteRead != size) {
		fprintf(stderr, "Error reading file.\n");
		free(bf);
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bf[size] = '\0';
	fclose(fp);
	Program *prog = (Program *)calloc(1, sizeof(Program));
	if (prog == NULL) {
		fprintf(stderr, "Memory allocation failed.\n");
		free(bf);
		exit(EXIT_FAILURE);
	}
	prog->lines = (ParseTreeNode **)calloc(
		1, sizeof(struct ParseTreeNode *) * 16384);
	if (prog->lines == NULL) {
		fprintf(stderr, "Memory allocation failed.\n");
		free(prog);
		free(bf);
		exit(EXIT_FAILURE);
	}

	Token *tokens =
		(Token *)calloc(1, sizeof(Token) * 25600);

	lexer(bf, tokens, 1);

	ParserContext context;
	context.tokens = tokens;
	context.tokenPtr = tokens;
	context.prog = prog;
	parse(&context);
	return 0;
}