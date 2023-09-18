#ifndef lexer_h
#define lexer_h

#include <stdbool.h>
#include <stdio.h>
#include "token_list.h"


static struct {
	char* begin;
	unsigned length;
} lexeme;

static struct {
	unsigned expr;
	unsigned line;
} tabs;

TokenList tokenize(FILE* source);

#endif