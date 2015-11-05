
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


/*
Shader* loadShader(char* path, GLenum type) {
	
	Shader* sh;
	char* source;
	
	sh = (Shader*)calloc(1, sizeof(Shader));
	
// 	sh->name = strdup(basename(path));
	sh->name = strdup("lol2\n");
	source = readFile(path);
	
	int sz = strlen(source);
	printf("read %d bytes from %s\n", sz, path);  
	
	sh->id = glCreateShader(type);
	glerr("shader create error");
	
	glShaderSource(sh->id, 1, (const char**)&source, NULL);
	glerr("shader source error");
	
	glCompileShader(sh->id);
	printLogOnFail(sh->id);
	glerr("shader compile error");

DONE:
	free(source);
	
	return sh;
}*/



void deleteShader(Shader* s) {
	
	
	
	
	
	
	
}

/*
ShaderProgram* loadProgram(char* vname, char* fname, char* gname, char* tcname, char* tname) {
	
	int bplen;
	char* vpath, *tcpath, *tpath, *gpath, *fpath, *fsub, *gsub, *vsub, *tsub, *tcsub;
	
	ShaderProgram* prog;
	
	if(!(vname || fname || gname || tname || tcname)) return NULL;

	prog = (ShaderProgram*)calloc(sizeof(ShaderProgram), 1);
	prog->id = glCreateProgram();
	
	tsub = "/tessellation/";
	tcsub = "/tescontrol/";
	vsub = "/vertex/";
	gsub = "/geometry/";
	fsub = "/fragment/";
	
	bplen = strlen(SHADER_BASE_PATH);
	
	if(vname) {
		vpath = (char*)malloc(bplen + strlen(vname) + strlen(vsub) + 6);
		sprintf(vpath, "%s%s%s.glsl", SHADER_BASE_PATH, vsub, vname);
		
		printf("Loading %s... \n", vpath);
		prog->vs = loadShader(vpath, GL_VERTEX_SHADER);
// 		printLogOnFail(prog->vs);
		
		glAttachShader(prog->id, prog->vs->id);
		glerr("Could not attach shader");
	}
	
	if(tcname) {
		tcpath = (char*)malloc(bplen + strlen(tcname) + strlen(tcsub) + 6);
		sprintf(tcpath, "%s%s%s.glsl", SHADER_BASE_PATH, tcsub, tcname);
		
		printf("Loading %s... \n", tcpath);
		prog->tcs = loadShader(tcpath, GL_TESS_CONTROL_SHADER);
// 		printLogOnFail(prog->vs);
		
		glAttachShader(prog->id, prog->tcs->id);
		glerr("Could not attach shader");
	}
	
	if(tname) {
		tpath = (char*)malloc(bplen + strlen(tname) + strlen(tsub) + 6);
		sprintf(tpath, "%s%s%s.glsl", SHADER_BASE_PATH, tsub, tname);
		
		printf("Loading %s... \n", tpath);
		prog->ts = loadShader(tpath, GL_TESS_EVALUATION_SHADER);
// 		printLogOnFail(prog->vs);
		
		glAttachShader(prog->id, prog->ts->id);
		glerr("Could not attach shader");
	
	}
	
	if(gname) {
		gpath = (char*)malloc(bplen + strlen(gname) + strlen(gsub) + 6);
		sprintf(gpath, "%s%s%s.glsl", SHADER_BASE_PATH, gsub, gname);
		
		printf("Loading %s... \n", gpath);
		prog->gs = loadShader(gpath, GL_GEOMETRY_SHADER);
// 		printLogOnFail(prog->vs);
		
		glAttachShader(prog->id, prog->gs->id);
		glerr("Could not attach shader");
	
	}
	
	
	if(fname) {
		fpath = (char*)malloc(bplen + strlen(fname) + strlen(fsub) + 6);
		sprintf(fpath, "%s%s%s.glsl", SHADER_BASE_PATH, fsub, fname);
		
		printf("Loading %s... \n", fpath);
		prog->fs = loadShader(fpath, GL_FRAGMENT_SHADER);
// 		printLogOnFail(prog->vs);
		
		glAttachShader(prog->id, prog->fs->id);
		glerr("Could not attach shader");
	}
	
	
	
	glLinkProgram(prog->id);
	glerr("linking program");

	

	
	return prog;
}

*/


GLenum nameToEnum(char* name) {
	
	if(0 == strcasecmp(name, "VERTEX")) return GL_VERTEX_SHADER;
	if(0 == strcasecmp(name, "TESS_CONTROL")) return GL_TESS_CONTROL_SHADER;
	if(0 == strcasecmp(name, "TESS_EVALUATION")) return GL_TESS_EVALUATION_SHADER;
	if(0 == strcasecmp(name, "GEOMETRY")) return GL_GEOMETRY_SHADER;
	if(0 == strcasecmp(name, "FRAGMENT")) return GL_FRAGMENT_SHADER;
	if(0 == strcasecmp(name, "COMPUTE")) return GL_COMPUTE_SHADER;
	
	return -1;
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
	
	base = strstr(source, "\n#shader");
	while(1) {
		
		// TODO skip line;
		int cnt = sscanf(base + 8, " %23s", &typeName); // != 1 for failure
		if(cnt == EOF || cnt == 0) {
			break;
		}
		
		//TODO check for EOF
		end = strstr(base + 8 + cnt, "\n#shader");
		
		type = nameToEnum(typeName);
		if(type == -1) {
			fprintf(stderr, "Invalid shader type %s in file \"%s\"\n", typeName, spath);
			base = end + 8;
			continue;
		}
		
		id = loadShaderSource(base, end - base, type);
		if(!id) { // BUG: look up the real failure code
			base = end + 8;
			continue;
		}
		
		glAttachShader(prog->id, id);
		glerr("Could not attach shader");
		
		base = end + 8;
		if(base >= source + srcLen) break;
	}
	
	free(source);
	free(spath);
	
	glLinkProgram(prog->id);
	glerr("linking program");
	
	return prog;
}
