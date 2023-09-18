#ifndef value_list_h
#define value_list_h

#include "value.h"

typedef struct {
	Value* list;
	unsigned capacity;
	unsigned it;
} ValueList;

#endif