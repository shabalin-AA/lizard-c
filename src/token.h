#ifndef token_h
#define token_h

typedef enum {
	lbrace_tok, rbrace_tok, call_tok,
	const_tok, id_tok, end_tok
} TokenKind;

typedef struct {
	TokenKind kind;
	char* spelling;
} Token;

void printToken(Token t);

void freeToken(Token* t);

#endif