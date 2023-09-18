#ifndef token_list_h
#define token_list_h

#include "token.h"

typedef struct {
	Token* list;
	unsigned capacity;
	unsigned it;
} TokenList;

#endif