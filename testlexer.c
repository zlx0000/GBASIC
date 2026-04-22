#include "gbasic.h"

int main(int argc, char **argv)
{
	FILE *fp = NULL;
	if (argc <= 1) {
		printf("Usage: gbasic ./file\n");
        return 0;
    }
	if (*argv[1]) {
		fp = fopen(argv[1], "r");
		if (fp == NULL) {
			perror("Can't open file.");
			exit(EXIT_FAILURE);
		}
	}

	fseek(fp, 0, SEEK_END);
	unsigned long size = ftell(fp);
	rewind(fp);

	char *bf = (char *)malloc(size + 1);
	if (bf == NULL) {
		perror("Can't load the file, memory allocation failed");
		fclose(fp);
		exit(EXIT_FAILURE);
	}

	size_t byteRead = fread(bf, 1, size, fp);
	if (byteRead != size) {
		perror("Error reading file.");
		free(bf);
		fclose(fp);
		exit(EXIT_FAILURE);
	}
	bf[size] = '\0';
	fclose(fp);
	Program *prog = (Program *)calloc(1, sizeof(Program));
	if (prog == NULL) {
		perror("Memory allocation failed.");
		free(bf);
		exit(EXIT_FAILURE);
	}
	prog->ParseTreeNode = (ParseTreeNode **)calloc(
		1, sizeof(ParseTreeNode *) * 16384);
	if (prog->ParseTreeNode == NULL) {
		perror("Memory allocation failed.");
		free(prog);
		free(bf);
		exit(EXIT_FAILURE);
	}

	Token *tokens =
		(Token *)calloc(1, sizeof(Token) * MAX_TOKEN);

    printf("%s\n", bf);
	int len = lexer(bf, tokens, 1);
	for (int i = 0; i < len; i++) printf("%d,%d: %s\n", tokens[i].lineNum, tokens[i].colNum, tokens[i].lexeme);
}