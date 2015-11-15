
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <dirent.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>


#include "utilities.h"



void glexit(char* msg) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "GL ERROR: %s: %s \n", msg, gluErrorString(err));
		exit(-1);
	}
}


char* glerr(char* msg) {
	char* errstr;
	GLenum err;
	
	err = glGetError();
	errstr = NULL; 
	
	if (err != GL_NO_ERROR) { 
		errstr = (char*)gluErrorString(err);
		fprintf(stderr, "GL ERROR: %s: %s \n", msg, errstr);
	}
	
	return errstr;
}




char* readFile(char* path, int* srcLen) {
	
	int fsize;
	char* contents;
	FILE* f;
	
	
	f = fopen(path, "rb");
	if(!f) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		return NULL;
	}
	
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	rewind(f);
	
	contents = (char*)malloc(fsize + 2);
	
	fread(contents+1, sizeof(char), fsize, f);
	contents[0] = '\n';
	contents[fsize] = 0;
	
	fclose(f);
	
	if(srcLen) *srcLen = fsize + 1;
	
	return contents;
}









