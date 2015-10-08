
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "utilities.h"
#include "shader.h"


const char* SHADER_BASE_PATH = "./shaders";



char* readFile(char* path) {
	
	int fsize;
	char* contents;
	FILE* f;
	
	
	f = fopen(path, "rb");
	
	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	rewind(f);
	
	contents = (char*)malloc(fsize + 1);
	
	fread(contents, sizeof(char), fsize, f);
	contents[fsize] = 0;
	
	fclose(f);
	
	return contents;
}



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
	
	glShaderSource(sh->id, 1, (const char**)&source, NULL);
	glerr("shader source error");
	
	glCompileShader(sh->id);
	glerr("shader compile error");

DONE:
	free(source);
	
	return sh;
}



void deleteShader(Shader* s) {
	
	
	
	
	
	
	
}


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
		
		glAttachShader(prog->id, prog->vs->id);
		glerr("Could not attach shader");
	}
	
	if(tcname) {
		tcpath = (char*)malloc(bplen + strlen(tcname) + strlen(tcsub) + 6);
		sprintf(tcpath, "%s%s%s.glsl", SHADER_BASE_PATH, tcsub, tcname);
		
		printf("Loading %s... \n", vpath);
		prog->tcs = loadShader(tcpath, GL_TESS_CONTROL_SHADER);
		
		glAttachShader(prog->id, prog->tcs->id);
		glerr("Could not attach shader");
	}
	
	if(tname) {
		tpath = (char*)malloc(bplen + strlen(tname) + strlen(tsub) + 6);
		sprintf(tpath, "%s%s%s.glsl", SHADER_BASE_PATH, tsub, tname);
		
		printf("Loading %s... \n", vpath);
		prog->ts = loadShader(tpath, GL_TESS_EVALUATION_SHADER);
		
		glAttachShader(prog->id, prog->ts->id);
		glerr("Could not attach shader");
	
	}
	
	if(gname) {
		gpath = (char*)malloc(bplen + strlen(gname) + strlen(gsub) + 6);
		sprintf(gpath, "%s%s%s.glsl", SHADER_BASE_PATH, gsub, gname);
		
		printf("Loading %s... \n", vpath);
		prog->gs = loadShader(gpath, GL_GEOMETRY_SHADER);
		
		glAttachShader(prog->id, prog->gs->id);
		glerr("Could not attach shader");
	
	}
	
	
	if(fname) {
		fpath = (char*)malloc(bplen + strlen(fname) + strlen(fsub) + 6);
		sprintf(fpath, "%s%s%s.glsl", SHADER_BASE_PATH, fsub, fname);
		
		printf("Loading %s... \n", vpath);
		prog->fs = loadShader(fpath, GL_FRAGMENT_SHADER);
		
		glAttachShader(prog->id, prog->fs->id);
		glerr("Could not attach shader");
	}
	
	
	
	glLinkProgram(prog->id);
	glerr("linking program");
	//glUseProgram(ProgramId);
	//gle("using program");
	

	
	return prog;
}
