
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "utilities.h"
#include "shader.h"


const char* SHADER_BASE_PATH = "./src/shaders/";



void printLogOnFail(id) {
	
	GLint success, logSize;
	GLsizei len;
	GLchar* log;
	
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if(success) return;
	
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logSize);
	
	log = (GLchar*)malloc(logSize);
	glGetShaderInfoLog(id, logSize, &len, log);
	
	fprintf(stderr, "Shader Log:\n%s", (char*)log);
	
	free(log);
}





GLuint loadShaderSource(char* source, int length, GLenum type) {
	
	GLuint id;
	
	id = glCreateShader(type);
	glerr("shader create error");
	
	glShaderSource(id, 1, (const char**)&source, &length);
	glerr("shader source error");
	
	glCompileShader(id);
	printLogOnFail(id);
	glerr("shader compile error");

	return id;
}





void deleteShader(Shader* s) {
	
	
	
	
	
	
	
}


GLenum nameToEnum(char* name) {
	
	if(0 == strcasecmp(name, "VERTEX")) return GL_VERTEX_SHADER;
	if(0 == strcasecmp(name, "TESS_CONTROL")) return GL_TESS_CONTROL_SHADER;
	if(0 == strcasecmp(name, "TESS_EVALUATION")) return GL_TESS_EVALUATION_SHADER;
	if(0 == strcasecmp(name, "GEOMETRY")) return GL_GEOMETRY_SHADER;
	if(0 == strcasecmp(name, "FRAGMENT")) return GL_FRAGMENT_SHADER;
	if(0 == strcasecmp(name, "COMPUTE")) return GL_COMPUTE_SHADER;
	
	return -1;
}



int extractShader(char** source, GLuint progID) {
	
	char* base, *end;
	int cnt;
	char typeName[24];
	GLenum type;
	GLuint id;
	
	base = strstr(*source, "\n#shader");
	if(base == NULL) {
		return 1; // no shaders left
	}
	
	
	cnt = sscanf(base + 8, " %23s", &typeName); // != 1 for failure
	if(cnt == EOF || cnt == 0) {
		return 2; // 
	}
	
	
	// skip #shader line
	base = strchr(base + 1, '\n');
	
	end = strstr(base, "\n#shader");
	if(end == NULL) {
		end = *source + strlen(*source);
	}
	
	printf(" %s", typeName); 
	
	*source = end;
	
	type = nameToEnum(typeName);
	if(type == -1) {
		fprintf(stderr, "Invalid shader type %s\n", typeName);
		return 3;
	}

	
	id = loadShaderSource(base, end - base, type);
	if(!id) { // BUG: look up the real failure code
		return 4;
	}
	
	glAttachShader(progID, id);
	glerr("Could not attach shader");
	
	return 0;
}



ShaderProgram* loadCombinedProgram(char* path) {
	
	ShaderProgram* prog;
	char* source, *spath, *end, *base;
	int srcLen;
	char typeName[24];
	GLenum type; 
	GLuint id;
	
	int bplen = strlen(SHADER_BASE_PATH);
	
	prog = calloc(1, sizeof(ShaderProgram));
	prog->id = glCreateProgram();
	
	// grab the source
	spath = (char*)malloc(bplen + strlen(path) + 6);
	sprintf(spath, "%s%s.glsl", SHADER_BASE_PATH, path);
	
	source = readFile(spath, &srcLen);
	if(!source) {
		free(spath);
		return NULL;
	}
	
	
	//parse out the contents one shader at a time
	
	base = source;
	int c = 0;
	
	printf("Loading %s:\n   ", spath);
	while(!extractShader(&base, prog->id));
	printf("\n");
	
	free(source);
	free(spath);
	
	glLinkProgram(prog->id);
	glerr("linking program");
	
	printLogOnFail(prog->id);
	
	return prog;
}






