#ifndef var_h
#define var_h

#include "value.h"

typedef struct {
	char* name;
	Value value;
	int scope;
} Var;

void printVar(Var v);
void freeVar(Var* v);

#endif