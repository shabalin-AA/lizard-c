#ifndef var_list_h
#define var_list_h

#include "var.h"

typedef struct {
	Var* list;
	unsigned capacity;
	unsigned it;
} VarList;

#endif