#ifndef list_h
#define list_h

#include <stdlib.h>

#define initList(_list, _type) \
	do { \
		(_list).it = 0; \
		(_list).capacity = 8; \
		(_list).list = malloc((_list).capacity * sizeof (_type)); \
	} while (0)
	
#define resize(_list, _type) \
	do { \
		(_list).capacity = 2 * (_list).capacity; \
		(_list).list = realloc((_list).list, (_list).capacity * sizeof(_type)); \
	} while (0)

#define push(_list, _type, _value) \
	do { \
		if ((_list).it >= (_list).capacity) \
			resize((_list), _type); \
		(_list).list[(_list).it++] = (_value); \
	} while (0)

#define pop(_list) (_list).list[--(_list).it]
		
#define tail(_list) (_list).list[(_list).it-1]
		
#define freeList(_list) free((_list).list)
		
#endif