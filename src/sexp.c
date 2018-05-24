


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sexp.h"


static char decode(char** s);



static sexp* parse_literal(char** s) {
	
	sexp* x;
	char* e;
	char q;
	int len, i;
	
	x = calloc(1, sizeof(*x));
	x->type = 1;
	
	//check if it's not quoted
	if(**s != '"' && **s != '\'') {
		
		e = strpbrk(*s, " \r\n\t)");
		if(!e) {
			fprintf(stderr, "sexp: unexpected end of input parsing literal\n");
			return x;
		}
		
		x->str = strndup(*s, e - *s);
		*s = e;
		return x;
	}
	
	// handled quoted strings
	q = **s;
	(*s)++;
	
	// find max length
	for(len = 0; (*s)[len] && (*s)[len] != q && (*s)[len - 1] != '\\'; len++);
	
	x->str = malloc(len + 1);
	
	i = 0;
	while(**s && **s != q && *(*s - 1) != '\\') {
		x->str[i] = decode(s);
		(*s)++;
		i++;
	}
	
	x->str[i] = '\0';
	
	return x;
}



static sexp* parse(char** s) {
	
	sexp* x, *y;
	
	x = calloc(1, sizeof(*x));
	x->type = 0;
	
	while(**s) {
		switch(**s) {
			case '(': // sub expression
				(*s)++;
				
				// TODO: check for (*   *) and skip as comment
				
				y = parse(s);
				VEC_PUSH(&x->args, y);
				break;
				
			case ')': // end of expression
				(*s)++;
				return x;
			
			case '\r': // skip whitespace
			case '\n':
			case '\t':
			case '\v':
			case ' ':
				(*s)++;
				break;
				
			default: // some literal of some sort
				y = parse_literal(s);
				VEC_PUSH(&x->args, y);
				break;
		}
	}
	
	fprintf(stderr, "sexp: unexpected end of input parsing expression.\n");
	
	return x;
}



sexp* sexp_parse(char* source) {
	char* s = source;
	
	return parse(&s);
}

sexp* sexp_parse_file(char* path) {
	char* s;
	
	s = readFile(path);
	
	return parse(&s);
}



void sexp_free(sexp* x) {
	int i;
	
	if(!x) return;
	
	for(i = 0; i < VEC_LEN(&x->args); i++) {
		sexp_free(VEC_ITEM(&x->args, i));
	}

	if(x->str) free(x->str);
	free(x);
}


























// out must be big enough, at least as big as in+1 just to be safe
// appends a null to out, but is also null-safe
static char decode(char** s) {

	char c = **s;
	
	if(c == '\\') {
		(*s)++;
		switch(**s) {
			case '\'': return '\'';  
			case '"': return '"';  
			case '`': return '`';  
			case '?': return '?';  
			case '0': return '\0';  
			case 'r': return '\r';  
			case 'n': return '\n'; 
			case 'f': return '\f'; 
			case 'a': return '\a'; 
			case 'b': return '\b'; 
			case 'v': return '\v'; 
			case 't': return '\t'; 
			case 'x': 
				// TODO: parse hex code
				return '?';
			case 'U':
				// TODO parse longer unicode
			case 'u': 
				// TODO: parse unicode
				return '?';
			// TODO: parse octal
				
			default:
				return '?';
		}
	}

	return c;
}




