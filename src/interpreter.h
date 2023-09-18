#ifndef interpreter_h
#define interpreter_h

#include "token_list.h"
#include "value_list.h"
#include "chunk.h"
#include "var_list.h"

typedef struct {
	int scope;
	int ip;
	ValueList valueStack;
	VarList varStack;
} Interpreter;

static void _interpret(Chunk c);
void interpret(Chunk chunk);

#endif