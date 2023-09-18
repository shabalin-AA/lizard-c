#include "lexer.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "token_list.h"
#include "list.h"


static bool numberLexeme() {
	char* c = lexeme.begin;
	if (!(isdigit(*c) || (*c == '-' && lexeme.length > 1))) return false;
	c++;
	for (unsigned i = 1; i < lexeme.length; i++, c++) {
		if (!isdigit(*c) && *c != '.') return false;
	}
	return true;
}

static bool isoperation(char c) {
	return 	(c == '+') || (c == '-') || (c == '/') || (c == '*') || 
					(c == '>') || (c == '<') || (c == '=');
}

static bool identifierLexeme() {
	char* c = lexeme.begin;
	if (lexeme.length == 1) {
		if (!(isoperation(*c) || (*c == '_') || isalpha(*c)))
			return false;
	}
	else {
		if (!isalpha(*c)) return false;
		for (unsigned i = 0; i < lexeme.length; i++, c++) {
			if (!(isalpha(*c) || 
					*c == '-' || 
					*c == '_' || 
					isdigit(*c)
			)) return false;
		}
	}
	return true;
}

static Token newToken() {
	Token temp;
	temp.spelling = malloc(lexeme.length+1);
	memcpy(temp.spelling, lexeme.begin, lexeme.length);
	temp.spelling[lexeme.length] = '\0';
	if (numberLexeme() || 
		(strcmp(temp.spelling, "true") == 0) || 
		(strcmp(temp.spelling, "false") == 0) || 
		(strcmp(temp.spelling, "nil") == 0)) 
	{
		temp.kind = const_tok;
	} 
	else if (identifierLexeme()) {
		temp.kind = id_tok;
	} 
	else {
		printf("ERROR: unexpected lexeme: '%s'\n", temp.spelling);
		exit(1);
	}
	return temp;
}

static void countTabs(char* line) {
	tabs.line = 1;
	for (unsigned i = 0; i < strlen(line); i++) {
		if (line[i] == '\t') tabs.line++;
		else break;
	}
}

static void tokenizeLine(char* line, TokenList* tokens) {
	lexeme.begin = line;
	lexeme.length = 0;
	countTabs(line);
	while (tabs.expr >= tabs.line) {
		push(*tokens, Token, (Token){.kind = end_tok});
		tabs.expr--;
	}
	for (unsigned i = 0; i < strlen(line); i++) {
		if (line[i] == '#') return;
		if (line[i] == ':') {
			if (lexeme.length > 0) push(*tokens, Token, newToken());
			push(*tokens, Token, (Token){.kind = call_tok});
			tabs.expr++;
			lexeme.begin++;
			lexeme.length = 0;
		}
		else if (line[i] == '[') {
			push(*tokens, Token, (Token){.kind = lbrace_tok});
			lexeme.begin++;
			lexeme.length = 0;
		}
		else if (line[i] == ']') {
			if (lexeme.length > 0) push(*tokens, Token, newToken());
			push(*tokens, Token, (Token){.kind = rbrace_tok});
			lexeme.begin++;
			lexeme.length = 0;
		}
		else if (isspace(line[i])) {
			if (lexeme.length > 0) push(*tokens, Token, newToken());
			lexeme.begin = &line[i+1];
			lexeme.length = 0;
		}
		else {
			lexeme.length++;
		}
	}
	if (lexeme.length > 0) push(*tokens, Token, newToken());
}

TokenList tokenize(FILE* source) {
	tabs.expr = 0;
	TokenList tokens;
	initList(tokens, Token);
	unsigned lineSize = 1024;
	char* line = malloc(lineSize);
	while (fgets(line, lineSize, source)) {
		tokenizeLine(line, &tokens);
	}
	strcpy(line, "\n");
	tokenizeLine(line, &tokens);
	free(line);
	return tokens;
}