#ifndef chunk_h
#define chunk_h

#include "token_list.h"

typedef struct {
	TokenList* tokens;
	int begin;
	int end;
} Chunk;

#endif