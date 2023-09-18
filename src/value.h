#ifndef value_h
#define value_h

#include <stdbool.h>
#include "func.h"

typedef enum {
	nil,
	number,
	boolean,
	nativeFunc,
	lambda,
	list
} ValueType;

void printValueType(ValueType type);

typedef struct {
	ValueType type;
	union {
		double number;
		bool boolean;
		void* object;
	} as;
	int scope;
} Value;

void printValue(Value v);
void freeValue(Value* v);

#endif