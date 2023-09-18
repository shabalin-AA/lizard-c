#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "interpreter.h"
#include "list.h"


int main(int argc, char** argv) {
	if (argc != 2) {
		printf("usage: ./liz file.liz\n");
		exit(1);
	}
	TokenList tokens = tokenize(fopen(argv[1], "r"));
	if (DEBUG) {
		for (unsigned i = 0; i < tokens.it; i++) {
			printf("%4d| ", i); 
			printToken(tokens.list[i]);
		}
	}
	interpret((Chunk) {
		.tokens = &tokens,
		.begin = 0,
		.end = tokens.it
	});
	for (unsigned i = 0; i < tokens.it; i++) {
		freeToken(&tokens.list[i]);
	}
	freeList(tokens);
	return 0;
}