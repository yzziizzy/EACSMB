 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "utilities.h"
#include "config.h"




// struct name, prop name, type
#define grabVal(s, p, t) \
	if(strcmp(str, #p) == 0) { \
		s->p = grab_##t(strchr(str, '=') + 1); \
		continue; \
	}


float grab_float(char* s) {
	return strtof(s, NULL);
}
double grab_double(char* s) {
	return strtod(s, NULL);
}
int64_t grab_int(char* s) {
	return strtol(s, NULL, 10);
}
int64_t grab_hex(char* s) {
	return strtol(s, NULL, 16);
}
char* grab_string(char* s) {
	size_t n;
	
	n = strspn(s, " \t");
	return strlndup(s + n);
}

UserConfig* loadConfigFile(char* path) {
	
	FILE* f;
	UserConfig* cfg;
	char str[2048];
	char* meh;
	
	
	f = fopen(path, "rb");
	if(!f) return NULL;
	
	cfg = calloc(1, sizeof(UserConfig));
	
	while(!feof(f)) {
		
		meh = fgets(str, 2048, f);
		if(!meh) return;
		
		if(str[0] == '#') continue;
		
		grabVal(cfg, scrollSpeed, float);
		
	}
	
	fclose(f);
	
	return cfg;
};



#undef grabVal

