#include "interpreter.h"
#include "token_list.h"
#include "value_list.h"
#include "value.h"
#include <string.h>
#include "list.h"
#include <stdlib.h>
#include "func.h"
#include <stdio.h>
#include "lambda.h"

static ValueList valueStack;
static VarList varStack;
static int ip;
static int scope;
static Chunk chunk;

static Lambda* getLambda(Value* v) {
	return ((Lambda*)v->as.object);
}

static NativeFunc getNativeFunc(Value* v) {
	return ((NativeFunc)v->as.object);
}

#define curToken (chunk.tokens->list[ip])

static Chunk subchunk(int begin, int end) {
	return (Chunk) {
		.tokens = chunk.tokens,
		.begin = begin,
		.end = end
	};
}

static int seekExprEnd(int begin) {
	int isEnd = 0;
	int i = begin;
	for (; isEnd != 1; i++) {
		if (chunk.tokens->list[i].kind == call_tok) {
			isEnd--;
		}
		if (chunk.tokens->list[i].kind == end_tok) {
			isEnd++;
		}
	}
	return i-1;
}

static void checkType(Value v, ValueType type) {
	if (v.type != type) {
		printf("ERROR: met unexpected type\nexpected: ");
		printValueType(type);
		printf("but given: ");
		printValueType(v.type);
		exit(1);
	}
}

static void lizAddFunc(void) {
	unsigned argBegin = valueStack.it;
	ip++;
	int end = seekExprEnd(ip);
	_interpret(subchunk(ip, end));
	for (unsigned i = argBegin; i < valueStack.it; i++) {
		Value val = pop(valueStack);
		checkType(val, number);
		checkType(tail(valueStack), number);
		tail(valueStack).as.number += val.as.number;
	}
	ip = end;
}

static void lizSubtractFunc(void) {
	unsigned argBegin = valueStack.it;
	ip++;
	int end = seekExprEnd(ip);
	_interpret(subchunk(ip, end));
	for (unsigned i = argBegin+2; i < valueStack.it; i++) {
		Value val = pop(valueStack);
		checkType(val, number);
		checkType(tail(valueStack), number);
		tail(valueStack).as.number += val.as.number;
	}
	Value val = pop(valueStack);
	checkType(val, number);
	checkType(tail(valueStack), number);
	tail(valueStack).as.number -= val.as.number;
	ip = end;
}

static void lizMultiplyFunc(void) {
	unsigned argBegin = valueStack.it;
	ip++;
	int end = seekExprEnd(ip);
	_interpret(subchunk(ip, end));
	for (unsigned i = argBegin; i < valueStack.it; i++) {
		Value val = pop(valueStack);
		checkType(val, number);
		checkType(tail(valueStack), number);
		tail(valueStack).as.number *= val.as.number;
	}
	ip = end;
}

static void lizDivideFunc(void) {
	unsigned argBegin = valueStack.it;
	ip++;
	int end = seekExprEnd(ip);
	_interpret(subchunk(ip, end));
	for (unsigned i = argBegin+2; i < valueStack.it; i++) {
		Value val = pop(valueStack);
		checkType(val, number);
		checkType(tail(valueStack), number);
		tail(valueStack).as.number *= val.as.number;
	}
	Value val = pop(valueStack);
	checkType(val, number);
	checkType(tail(valueStack), number);
	tail(valueStack).as.number /= val.as.number;
	ip = end;
}

#define max(v1, v2) (v1) > (v2) ? (v1) : (v2)

static void lizGrFunc(void) {
	unsigned argBegin = valueStack.it;
	ip++;
	int end = seekExprEnd(ip);
	_interpret(subchunk(ip, end));
	for (unsigned i = argBegin+2; i < valueStack.it; i++) {
		Value val = pop(valueStack);
		checkType(val, number);
		checkType(tail(valueStack), number);
		tail(valueStack).as.number = max(tail(valueStack).as.number, val.as.number);
	}
	Value val = pop(valueStack);
	checkType(val, number);
	checkType(tail(valueStack), number);
	tail(valueStack).as.boolean = tail(valueStack).as.number > val.as.number;
	tail(valueStack).type = boolean;
	ip = end;
}

#undef max

#define min(v1, v2) (v1) < (v2) ? (v1) : (v2)

static void lizLsFunc(void) {
	unsigned argBegin = valueStack.it;
	ip++;
	int end = seekExprEnd(ip);
	_interpret(subchunk(ip, end));
	for (unsigned i = argBegin+2; i < valueStack.it; i++) {
		Value val = pop(valueStack);
		checkType(val, number);
		checkType(tail(valueStack), number);
		tail(valueStack).as.number = min(tail(valueStack).as.number, val.as.number);
	}
	Value val = pop(valueStack);
	checkType(val, number);
	checkType(tail(valueStack), number);
	tail(valueStack).as.boolean = tail(valueStack).as.number < val.as.number;
	tail(valueStack).type = boolean;
	ip = end;
}

#undef min

static bool valueEquals(Value v1, Value v2) {
	if (v1.type == boolean && v2.type == boolean) return v1.as.boolean == v2.as.boolean;
	if (v1.type == number && v2.type == number) return v1.as.number == v2.as.number;
	if (v1.type == nil && v2.type == nil) return true;
	return false;
}

static void lizEqFunc(void) {
	unsigned argBegin = valueStack.it;
	ip++;
	int end = seekExprEnd(ip);
	_interpret(subchunk(ip, end));
	bool eq = true;
	for (unsigned i = argBegin; i < valueStack.it; i++) {
		Value val = pop(valueStack);
		checkType(val, number);
		checkType(tail(valueStack), number);
		eq = eq && valueEquals(tail(valueStack), val);
	}
	tail(valueStack).as.boolean = eq;
	tail(valueStack).type = boolean;
	ip = end;
}

static void lizPrintFunc(void) {
	ip++;
	int end = seekExprEnd(ip);
	unsigned argBegin = valueStack.it;
	_interpret(subchunk(ip, end));
	for (unsigned i = argBegin; i < valueStack.it; i++) {
		printValue(valueStack.list[i]);
	}
	ip = end;
}

static void lizDefFunc(void) {
	ip++;
	char* varName;
	if (curToken.kind == id_tok) {
		varName = curToken.spelling;
	}
	else {
		printf("ERROR: expected identifier for def\n");
		exit(1);
	}
	ip++;
	int end = seekExprEnd(ip);
	_interpret(subchunk(ip, end));
	Value varValue = pop(valueStack);
	Var newVar = (Var) {
		.name = varName,
		.value = varValue,
		.scope = scope
	};
	push(varStack, Var, newVar);
	ip = end;
}

static void lizListFunc(void) {
	ip++;
	int end = seekExprEnd(ip);
	Value newList = (Value) {
		.type = list,
		.as.object = malloc(sizeof(ValueList)),
		.scope = scope
	};
	ValueList* l = (ValueList*)newList.as.object;
	initList(*l, Value);
	unsigned argBegin = valueStack.it;
	_interpret(subchunk(ip, end));
	for (unsigned i = argBegin; i < valueStack.it; i++) {
		Value v = valueStack.list[i];
		push(*l, Value, v);
	}
	for (unsigned i = argBegin; i < valueStack.it; i++) {
		pop(valueStack);
	}
	push(valueStack, Value, newList);
	ip = end;
}

static void checkArgc(int argBegin, int argcExpected) {
	int argcActual = valueStack.it - argBegin;
	if (argcActual != argcExpected) {
		printf("ERROR: wrong number of arguments passed. expected %d but given %d\n", argcExpected, argcActual);
		exit(1);
	}
}

static void lizAppendFunc(void) {
	ip++;
	int end = seekExprEnd(ip);
	unsigned argBegin = valueStack.it;
	_interpret(subchunk(ip, end));
	checkArgc(argBegin, 2);
	Value toAppend = pop(valueStack);
	checkType(tail(valueStack), list);
	ValueList* l = (ValueList*) tail(valueStack).as.object;
	push(*l, Value, toAppend);
	ip = end;
}

static void lizPopFunc(void) {
	ip++;
	int end = seekExprEnd(ip);
	unsigned argBegin = valueStack.it;
	_interpret(subchunk(ip, end));
	checkArgc(argBegin, 1);
	checkType(tail(valueStack), list);
	ValueList* l = (ValueList*) tail(valueStack).as.object;
	Value v = pop(*l);
	push(valueStack, Value, v);
	ip = end;
}

static void lizNthFunc(void) {
	ip++;
	int end = seekExprEnd(ip);
	unsigned argBegin = valueStack.it;
	_interpret(subchunk(ip, end));
	checkArgc(argBegin, 2);
	Value indexValue = pop(valueStack);
	checkType(indexValue, number);
	Value listValue = tail(valueStack);
	checkType(listValue, list);
	ValueList* list = (ValueList*)listValue.as.object;
	int ind = (int)indexValue.as.number;
	if (ind >= 0 && ind < (int)list->it) {
		push(valueStack, Value, list->list[ind]);
	}
	else {
		Value nilValue = (Value) {
			.type = nil,
			.as.boolean = nil,
			.scope = scope
		};
		push(valueStack, Value, nilValue);
	}
	ip = end;
}

static void lizLambdaFunc(void) {
	ip++;
	int end = seekExprEnd(ip);
	Lambda l = subchunk(ip, end);
	Value v = (Value) {
		.type = lambda,
		.as.object = malloc(sizeof(Lambda)),
		.scope = scope
	};
	*(Lambda*)v.as.object = l;
	push(valueStack, Value, v);
	ip = end;
}

static void callLambda(Lambda* l) {
	ip++;
	int end = seekExprEnd(ip);
	unsigned argBegin = valueStack.it;
	_interpret(subchunk(ip, end));
	if (l->tokens->list[l->begin].kind != lbrace_tok) {
		printf("ERROR: expected arguments definition for lambda\n");
		exit(1);
	}
	int argsEnd = -1;
	for (int i = l->begin; i < l->end; i++) {
		if (l->tokens->list[i].kind == rbrace_tok) {
			argsEnd = i;
			break;
		}
	}
	if (argsEnd == -1) {
		printf("ERROR: unmatched ']' in arguments definition\n");
		exit(1);
	}
	checkArgc(argBegin, argsEnd - l->begin - 1);
	for (int i = l->begin + 1, j = 0; i < argsEnd; i++, j++) {
		if (l->tokens->list[i].kind != id_tok) {
			printf("ERROR: expected arguments definition for lambda\n");
			exit(1);
		}
		Var arg = (Var) {
			.name = l->tokens->list[i].spelling,
			.value = valueStack.list[argBegin+j],
			.scope = scope
		};
		push(varStack, Var, arg);
	}
	_interpret(subchunk(argsEnd+1, l->end));
	ip = end;
}

static void lizMapFunc(void) {
	printf("map is not implemented\n");
	exit(1);
	ip++;
	int end = seekExprEnd(ip);
	unsigned argBegin = valueStack.it;
	_interpret(subchunk(ip, end));
	checkArgc(argBegin, 2);
	Value processFunc = pop(valueStack);
	checkType(processFunc, lambda);
	Lambda* lambda = (Lambda*)processFunc.as.object 
	Value listValue = tail(valueStack);
	checkType(listValue, list);
	ValueList* list = (ValueList*)listValue.as.object;
	for (unsigned i = 0; i < 1; i++) {
		argBegin = valueStack.it;
		printValue(list->list[i]);
		push(valueStack, Value, list->list[i]);
		//
		checkArgc(argBegin, 1);
		printValue(tail(valueStack));
		list->list[i] = pop(valueStack);
	}
	ip = end;
}

static Value newValue(Token t) {
	if (strcmp(t.spelling, "true") == 0) {
		return (Value) {
			.type = boolean,
			.as.boolean = true,
			.scope = scope
		};
	}
	if (strcmp(t.spelling, "false") == 0) {
		return (Value) {
			.type = boolean,
			.as.boolean = false,
			.scope = scope
		};
	}
	if (strcmp(t.spelling, "nil") == 0) {
		return (Value) {
			.type = nil,
			.as.boolean = false,
			.scope = scope
		};
	}
	return (Value) {
		.type = number, 
		.as.number = atof(t.spelling), 
		.scope = scope
	};
}

static Value getVarValue(Token t) {
	for (unsigned i = varStack.it-1; i >= 0; i--) {
		if (strcmp(t.spelling, varStack.list[i].name) == 0) {
			// printf("%s found in variables\n", t.spelling);
			return varStack.list[i].value;
		}
	}
	// printf("%s not found in variables\n", t.spelling);
	return (Value) {
		.type = nil,
		.as.boolean = false,
		.scope = 1
	};
}

static Value getNextValue() {
	Value val;
	if (chunk.tokens->list[ip+1].kind == call_tok) {
		int end = seekExprEnd(ip + 2);
		_interpret(subchunk(ip, end));
		val = pop(valueStack);
		ip = end;
	}
	else {
		if (curToken.kind == const_tok) {
			val = newValue(curToken);
		}
		else {
			val = getVarValue(curToken);
		}
	}
	ip++;
	return val;
}

static void skipNextExpr() {
	if (chunk.tokens->list[ip+1].kind == call_tok) {
		int end = seekExprEnd(ip + 2);
		ip = end;
		return;
	}
	ip++;
}

static void lizIfFunc(void) {
	ip++;
	int end = seekExprEnd(ip);
	// if predicate is expression
	Value pred = getNextValue();
	checkType(pred, boolean);
	Value res;
	if (pred.as.boolean) {
		res = getNextValue();
		skipNextExpr();
	}
	else {
		skipNextExpr();
		res = getNextValue();
	}
	push(valueStack, Value, res);
	ip = end;
}

static void collectValues(int argBegin) {
	if (valueStack.it == 0) {
		Value nilValue = (Value) {
			.type = nil,
			.as.boolean = false,
			.scope = scope
		};
		push(valueStack, Value, nilValue);
		return;
	}
	Value ret = pop(valueStack);
	for (int i = argBegin; i < (int)valueStack.it; i++) {
		Value* v = &pop(valueStack);
		if (v->scope >= scope) {
			freeValue(v);
		}
	}
	push(valueStack, Value, ret);
	if (DEBUG) {
		printf("return: ");
		printValue(ret);
	}
}

static void collectVars() {
	if (varStack.it == 0) return;
	for (int i = varStack.it-1; i >= 0 && varStack.list[i].scope >= scope; i--) {
		Var* v = &pop(varStack);
		freeVar(v);
	}
}

static void GC(int argBegin) {
	if (DEBUG) printf("\n#### GC scope %d ####\n", scope);
	collectValues(argBegin);
	collectVars();
}

static void callFunc(Value func) {
	if (func.type == lambda) {
		Lambda* l = getLambda(&func);
		scope++;
		int argBegin = valueStack.it;
		callLambda(l);
		GC(argBegin);
		scope--;
	}
	else if (func.type == nativeFunc) {
		getNativeFunc(&func)();
	}
}

static void _interpret(Chunk c) {
	if (DEBUG) printf("\n#### interpreter scope %d ####\n", scope);
	Chunk chunk_backup = chunk;
	chunk = c;
	if (DEBUG) printf("chunk %d : %d\n", chunk.begin, chunk.end);
	int ip_backup = ip;
	for (ip = chunk.begin; ip < chunk.end; ip++) {
		// curToken is a macro
		if (curToken.kind == const_tok) {
			push(valueStack, Value, newValue(curToken));
		}
		else if (curToken.kind == id_tok) {
			push(valueStack, Value, getVarValue(curToken));
		}
		else if (curToken.kind == call_tok) {
			Value func = pop(valueStack);
			callFunc(func);
		}
		else {
			exit(1);
		}
	}
	ip = ip_backup;
	chunk = chunk_backup;
}

static Var initNativeFunc(char* name, NativeFunc f) {
	return (Var) {
		.name = name,
		.scope = 0,
		.value = ((Value) {
			.type = nativeFunc,
			.as.object = f,
			.scope = 0
		})
	};
}

static void initInter() {
	initList(valueStack, Value);
	initList(varStack, Var);
	ip = 0;
	scope = 1;

	push(varStack, Var, initNativeFunc("map", 		lizMapFunc));
	push(varStack, Var, initNativeFunc("nth", 		lizNthFunc));
	push(varStack, Var, initNativeFunc("list", 		lizListFunc));
	push(varStack, Var, initNativeFunc("append", 	lizAppendFunc));
	push(varStack, Var, initNativeFunc("pop", 		lizPopFunc));
	push(varStack, Var, initNativeFunc("+", 			lizAddFunc));
	push(varStack, Var, initNativeFunc("-", 			lizSubtractFunc));
	push(varStack, Var, initNativeFunc("*", 			lizMultiplyFunc));
	push(varStack, Var, initNativeFunc("/", 			lizDivideFunc));
	push(varStack, Var, initNativeFunc(">", 			lizGrFunc));
	push(varStack, Var, initNativeFunc("<", 			lizLsFunc));
	push(varStack, Var, initNativeFunc("=", 			lizEqFunc));
	push(varStack, Var, initNativeFunc("def", 		lizDefFunc));
	push(varStack, Var, initNativeFunc("if", 			lizIfFunc));
	push(varStack, Var, initNativeFunc("print", 	lizPrintFunc));
	push(varStack, Var, initNativeFunc("lambda", 	lizLambdaFunc));
}

static void freeInter() {
	freeList(valueStack);
	freeList(varStack);
}

void interpret(Chunk chunk) {
	initInter();
	_interpret(chunk);
	freeInter();
}
