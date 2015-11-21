
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



void _glexit(char* msg, char* file, int line, char* func) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "GL ERROR at %s:%d (%s): %s: %s \n", file, line, func, msg, gluErrorString(err));
		exit(-1);
	}
}


char* _glerr(char* msg, char* file, int line, char* func) {
	char* errstr;
	GLenum err;
	
	err = glGetError();
	errstr = NULL; 
	
	if (err != GL_NO_ERROR) { 
		errstr = (char*)gluErrorString(err);
		fprintf(stderr, "GL ERROR at %s:%d (%s): %s: %s \n", file, line, func, msg, errstr);
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




GLuint makeVAO(VAOConfig* details, int stride) {
	int i, offset; // packed data is expected
	GLuint vao;
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	for(i = 0; details[i].sz != 0; i++) {
		GLenum t;
		int ds;
		
		glEnableVertexAttribArray(i);
		
		t = details[i].type;
		if(t == GL_FLOAT) { // works only for my usage
			
			glVertexAttribPointer(i, details[i].sz, t, GL_FALSE, stride, (void*)offset);
		}
		else {
			glVertexAttribIPointer(i, details[i].sz, t, stride, (void*)offset);
		}
		glerr("vao init");
		
		if(t == GL_UNSIGNED_BYTE || t == GL_BYTE) ds = 1;
		else if(t == GL_UNSIGNED_SHORT || t == GL_SHORT) ds = 2;
		else ds = 4;
		
		offset += ds * details[i].sz;
	}
	
	return vao;
}




