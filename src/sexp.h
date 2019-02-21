#ifndef __EACSMB_sexp_h__
#define __EACSMB_sexp_h__




#include "ds.h"





struct sexp;

typedef struct sexp {
	char type;
	char brace;
	char* str;
	VEC(struct sexp*) args;
	
} sexp;




sexp* sexp_parse(char* source);
void sexp_free(sexp* x);

int64_t sexp_asInt(sexp* x); 
double sexp_asDouble(sexp* x); 

int64_t sexp_argAsInt(sexp* x, int argn);
double sexp_argAsDouble(sexp* x, int argn);
char* sexp_argAsStr(sexp* x, int argn); 
sexp* sexp_argAsSexp(sexp* x, int argn); 


#endif // __EACSMB_sexp_h__
