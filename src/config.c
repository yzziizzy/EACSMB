 
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
	if(strncmp(str, #p, strlen(#p)) == 0) { \
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
	
	UserConfig* cfg = malloc(sizeof(UserConfig));
	int status;
	
	zeroConfig(cfg);
	status = updateConfigFromFile(cfg, path);
	
	if(status) {
		free(cfg);
		cfg = NULL;
	}
	
	return cfg;
};


int updateConfigFromFile(UserConfig* config, char* path) {
	
	FILE* f;
	char str[2048];
	char* meh;
	
	if (config == NULL) return 1;
	
	if (path == NULL) return 2;
	
	f = fopen(path, "rb");
	if(!f) return 3;
	
	while(!feof(f)) {
		
		meh = fgets(str, 2048, f);
		if(!meh) return;
		
		if(str[0] == '#') continue;
		
		grabVal(config, keyRotateSensitivity, float);
		grabVal(config, keyScrollSensitivity, float);
		grabVal(config, keyZoomSensitivity, float);

		grabVal(config, mouseRotateSensitivity, float);
		grabVal(config, mouseScrollSensitivity, float);
		grabVal(config, mouseZoomSensitivity, float);
		
	}
	
	fclose(f);
	
	return 0;
	
}


void zeroConfig(UserConfig* config) {
	
	config->keyRotateSensitivity = 0.0f;
	config->keyScrollSensitivity = 0.0f;
	config->keyZoomSensitivity   = 0.0f;

	config->mouseRotateSensitivity = 0.0f;
	config->mouseScrollSensitivity = 0.0f;
	config->mouseZoomSensitivity   = 0.0f;
	
}


#undef grabVal

