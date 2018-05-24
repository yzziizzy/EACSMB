




#include "ds.h"





struct sexp;

typedef struct sexp {
	char type;
	char* str;
	VEC(struct sexp*) args;
	
} sexp;




sexp* sexp_parse(char* source);



