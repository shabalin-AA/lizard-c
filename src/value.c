#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "value_list.h"
#include "lambda.h"

void printValueType(ValueType type) {
	switch (type) {
		case nil: 				printf("nil\n"); break;
		case number: 			printf("number\n"); break;
		case boolean:			printf("boolean\n"); break;
		case nativeFunc: 	printf("nativeFunc\n"); break;
		case lambda: 			printf("lambda\n"); break;
		case list: 				printf("list\n"); break;
	}
}

void printValue(Value v) {
	if (DEBUG) printf("Value of type ");
	if (DEBUG) printValueType(v.type);
	switch (v.type) {
		case nil: 				printf("nil\n"); break;
		case number: 			printf("%g\n", v.as.number); break;
		case boolean: 		printf("%s\n", v.as.boolean == true ? "true" : "false"); break;
		case nativeFunc: 	printf("[Native Function]\n"); break;
		case lambda: {
			Lambda* l = (Lambda*)v.as.object;
			printf("[lambda %d : %d]\n", l->begin, l->end);
			break;
		}
		case list: {
			ValueList* l = (ValueList*)v.as.object;
			printf("list: [\n");
			for (unsigned i = 0; i < l->it; i++) {
				printf("  ");	
				printValue(l->list[i]);
			}
			printf("]\n");
			break;
		}
	}
	if (DEBUG) printf("scope: %d\n", v.scope);
}

void freeValue(Value* v) {
	if (DEBUG) {
		printf("destructing ");
		printValue(*v);
	}
	if (v->type == list) {
		ValueList* l = (ValueList*)v->as.object;
		freeList(*l);
		free(v->as.object);
	}
	else if (v->type == lambda) {
		free(v->as.object);
	}
}