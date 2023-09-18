#include "token.h"
#include <stdio.h>
#include <stdlib.h>

void printToken(Token t) {
	switch (t.kind) {
		case lbrace_tok: printf("[\n"); break;
		case rbrace_tok: printf("]\n"); break;
		case call_tok: printf("call\n"); break;
		case const_tok: 
			printf("constant '%s'\n", t.spelling);
			break;
		case id_tok: 
			printf("identifier '%s'\n", t.spelling); 
			break;
		case end_tok: printf("end\n"); break;
	}
}

void freeToken(Token* t) {
	free(t->spelling);
}