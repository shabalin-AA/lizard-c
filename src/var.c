#include "var.h"
#include <stdio.h>
#include "value.h"
#include <stdlib.h>

void printVar(Var v) {
	printf("Variable %s\n", v.name);
	printValue(v.value);
	printf("scope: %d\n", v.scope);
}

void freeVar(Var* v) {
	if (DEBUG) {
		printf("destructing ");
		printVar(*v);
	}
	// free(v->name); no need to free name because it points to token spelling and it will be freed im main()
	if (v->value.scope >= v->scope) {
		freeValue(&v->value);
	}
}