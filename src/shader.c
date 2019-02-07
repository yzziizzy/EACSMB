
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <libgen.h>

#include "utilities.h"
#include "shader.h"
#include "hash.h"


/*
TODO:
clean up debug file/line info
capture compile errors and translat to actual file and line
*/

static char* SHADER_BASE_PATH = NULL;

void Shader_setGlobalShaderDir(char* path) {
	SHADER_BASE_PATH = path;
}


typedef VEC(char*) stringlist;

typedef struct {
	char* src;
	char* file_path;
	int file_line;
} LineInfo;

typedef struct ShaderSource {
	VEC(LineInfo) lines;
	VEC(char*) strings; // only used in final separated shaders
	char* path;
	
	GLenum type;
	uint32_t version;
	GLuint id;
	
} ShaderSource;

static void strsplit(char* source, stringlist* out);

static void printLogOnFail(GLuint id, ShaderSource* ss) {
	
	GLint success, logSize;
	GLsizei len;
	GLchar* log;
	
	if(!glIsShader(id)) {
		fprintf(stderr, "id is not a shader!\n");
		return;
	}
	
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if(success) return;
	
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logSize);
	log = (GLchar*)malloc(logSize);
	
	glGetShaderInfoLog(id, logSize, &len, log);
//	fprintf(stderr, "Shader Log:\n%s", (char*)log);
	
	// parser for nvidia drivers on linux:
	stringlist l;
	VEC_INIT(&l);
	
	strsplit(log, &l);
	
	VEC_EACH(&l, si, s) {
		int msgOff;
		int lineNum, strNum;
		
		// this is broken somewhere. the line numbers from the driver are randomly wrong
		sscanf(s, "%d(%d) : %n", &strNum, &lineNum, &msgOff);
		printf("line: %d, err: '%s'", lineNum, s + msgOff);
		LineInfo* li = &VEC_ITEM(&ss->lines, lineNum - 1);
		printf("source: %s:%d : %s\n", li->file_path, li->file_line, li->src);
		printf("string source: %s\n", VEC_ITEM(&ss->strings, lineNum - 1));
	}
	
	printf("aborting due to shader error\n");
	exit(1);
	free(log);
}

static void printProgLogOnFail(id) {
	
	GLint success, logSize;
	GLsizei len;
	GLchar* log;
	
	if(!glIsProgram(id)) {
		fprintf(stderr, "id is not a program!\n");
		return;
	}
	
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if(success) return;
	
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logSize);
	log = (GLchar*)malloc(logSize);
	
	glGetProgramInfoLog(id, logSize, &len, log);
	fprintf(stderr, "Program Log:\n%s", (char*)log);
	
	free(log);
}


static GLenum indexToEnum(int index) {
	GLenum a[] = {
		GL_VERTEX_SHADER,
		GL_TESS_CONTROL_SHADER,
		GL_TESS_EVALUATION_SHADER,
		GL_GEOMETRY_SHADER,
		GL_FRAGMENT_SHADER,
		GL_COMPUTE_SHADER
	};
	
	if(index < 0 || index > 5) return -1;
	
	return a[index];
}


static int nameToIndex(char* name) {
	
	if(0 == strcasecmp(name, "VERTEX")) return 0;
	if(0 == strcasecmp(name, "TESS_CONTROL")) return 1;
	if(0 == strcasecmp(name, "TESS_EVALUATION")) return 2;
	if(0 == strcasecmp(name, "GEOMETRY")) return 3;
	if(0 == strcasecmp(name, "FRAGMENT")) return 4;
	if(0 == strcasecmp(name, "COMPUTE")) return 5;
	
	return -1;
}

static GLenum nameToEnum(char* name) {
	
	if(0 == strcasecmp(name, "VERTEX")) return GL_VERTEX_SHADER;
	if(0 == strcasecmp(name, "TESS_CONTROL")) return GL_TESS_CONTROL_SHADER;
	if(0 == strcasecmp(name, "TESS_EVALUATION")) return GL_TESS_EVALUATION_SHADER;
	if(0 == strcasecmp(name, "GEOMETRY")) return GL_GEOMETRY_SHADER;
	if(0 == strcasecmp(name, "FRAGMENT")) return GL_FRAGMENT_SHADER;
	if(0 == strcasecmp(name, "COMPUTE")) return GL_COMPUTE_SHADER;
	
	return -1;
}





// does not remove \n chars. gl wants them.
static void strsplit(char* source, stringlist* out) { 
	char* s = source;
	char* lstart = source;
	
	while(*s) {
		if(*s == '\n') {
			VEC_PUSH(out, strndup(lstart, s - lstart + 1));
			lstart = s + 1;
		}
		s++;
	}
	
	// handle the last line
	if(s > lstart) {
		VEC_PUSH(out, strndup(lstart, s - lstart));
	}
}

static ShaderSource* makeShaderSource() {
	ShaderSource* n;
	
	n = calloc(1, sizeof(*n));
	CHECK_OOM(n);
	
	VEC_INIT(&n->lines);
	VEC_INIT(&n->strings);
}

static char* extractFileName(char* src) {
	char* path, *s, *e;
	int delim;
	
	s = src;
	
	// skip spaces
	while(*s && *s == ' ') s++;
	
	delim = *s++;
	e = strchr(s, delim);
	path = strndup(s, e - s);
	
	return path;
}

static char* realFromSiblingPath(char* sibling, char* file) {
	char* falsePath, *realPath;
	
	char* fuckdirname = strdup(sibling);
	char* dir;
	
	dir = dirname(fuckdirname);
	
	falsePath = pathJoin(dir, file);
	
	realPath = realpath(falsePath, NULL);
	if(!realPath) {
		// handle errno
	}
	
	free(fuckdirname);
	free(falsePath);
	
	return realPath;
}

static void ss_loadFile(ShaderSource* ss, char* path) {
	stringlist l;
	int i;
	char* source, *s;
	char* includeFileName, *includeFilePath;
	
	
	source = readFile(path, NULL);
	if(!source) {
		// TODO copypasta, fix this
		fprintf(stderr, "failed to load shader file '%s'\n", path);
		return NULL;
	}
	
	VEC_INIT(&l);
	strsplit(source, &l);
	
	
	VEC_EACH(&l, lii, li) {
		
		if(0 == strncmp(li, "#include", strlen("#include"))) {
			// parse include
			s = li + strlen("#include");
			
			includeFileName = extractFileName(s);
			includeFilePath = realFromSiblingPath(path, includeFileName);
			
			ss_loadFile(ss, includeFilePath);
		}
		else {
			LineInfo* qli;
			VEC_INC(&ss->lines);
// 			printf("line %d: %s", lii, li);
			qli = &VEC_TAIL(&ss->lines);
			qli->src = li;
			qli->file_path = path;
			qli->file_line = lii;
		}
	}
	
	VEC_FREE(&l);
}

ShaderSource* loadShaderSource(char* path) {
	ShaderSource* ss;
	stringlist l;
	int i;
	char* source, *s;
	char* includeFileName, *includeFilePath;
	
	VEC_INIT(&l);
	ss = makeShaderSource();
	
	ss->path = path;
	
	ss_loadFile(ss, path);
	
	return ss;
}




/*
void processIncludes(ShaderProgram* sp, ShaderSource* ss) {
	int i;
	char* s, *includeFileName, *includeFilePath;
	
	for(i = 0; i < VEC_LEN(&ss->lines); i++) {
		ShaderSource* iss;
		LineInfo* li = &VEC_ITEM(&ss->lines, i);
		
		
		if(0 == strncmp("#include", li->src, strlen("#include"))) {
			s = li->src + strlen("#include");
			
			includeFileName = extractFileName(s);
			includeFilePath = realFromSiblingPath(ss->path, includeFileName);
			
			// TODO: recursion detection
			iss = loadShaderSource(includeFilePath);
			
			//HT_set(&sp->sources, includeFilePath, iss);
			
			// insert the included lines into this file's lines
			VEC_SPLICE(&ss->lines, &iss->lines, i+1);
			
			// comment out this line
			li->src[0] = '/';
			li->src[1] = '/';
			
			// the spliced in lines are ahead of the loop counter.
			// recursion is not necessary
		}
		
	}
}
*/

void extractShaders(ShaderProgram* sp, ShaderSource* raw) {
	int i;
	char* s;
	char typeName[24];
	int commonVersion = 0;
	
	ShaderSource* curShader = NULL;
	
	
	ShaderSource* common;
	common = makeShaderSource();
	
	curShader = common;
	
	
	for(i = 0; i < VEC_LEN(&raw->lines); i++) {
		int cnt, index;
		int version;
		LineInfo* li = &VEC_ITEM(&raw->lines, i);
		
		
		// extract the GLSL version to be prepended to the shader
		if(0 == strncmp("#version", li->src, strlen("#version"))) {
			s = li->src + strlen("#version");
			
			sscanf(s, " %d", &version);
			
			curShader->version = version;
			
			// version lines are not copied over, free them now
			free(li->src);
		}
		else if(0 == strncmp("#shader", li->src, strlen("#shader"))) {
			s = li->src + strlen("#shader");
			
			cnt = sscanf(s, " %23s", typeName); // != 1 for failure
			if(cnt == EOF || cnt == 0) {
				printf("failure scanning shader type name\n");
				continue;
			}
			
			index = nameToIndex(typeName);
			
			sp->shaders[index] = curShader = makeShaderSource();
			curShader->type = indexToEnum(index);
			
			// shader lines are not copied over, free them now
			free(li->src);
		}
		else {
			// copy line 
			VEC_PUSH(&curShader->lines, *li);
		}
		
	}
	
	
	// fill in common info
	for(i = 0; i < 6; i++) {
		ShaderSource* ss = sp->shaders[i];
		char* s;
		
		if(!ss) continue;
		
		if(!ss->version || ss->version < common->version) ss->version = common->version;
		
		// TODO: default version?
		
		// make space at the front
		VEC_RESERVE(&ss->lines, VEC_LEN(&common->lines) + 1, 0);
		
		// add the version line
		s = malloc(20);
		sprintf(s, "#version %0.3u\n", ss->version);
		VEC_ITEM(&ss->lines, 0).src = s;
		VEC_ITEM(&ss->lines, 0).file_path = NULL;
		VEC_ITEM(&ss->lines, 0).file_line = -1;
		
		// add the common lines
		VEC_OVERWRITE(&ss->lines, &common->lines, 1);
		
		// fill in strings
		VEC_INIT(&ss->strings);
		VEC_EACH(&ss->lines, lii, li) {
			VEC_PUSH(&ss->strings, li.src);
			
// 			printf("%d [%d] %s:%d: %s", lii, i, li.file_path, li.file_line, li.src );
		}
	}
	

	
}



static ShaderProgram* makeShaderProgram() {
	ShaderProgram* sp;
	
	sp = calloc(1, sizeof(*sp));
	CHECK_OOM(sp);
	
	HT_init(&sp->sources, 0);
	
	return sp;
}

static void compileShader(ShaderSource* ss) {
	GLuint id;
	glerr("pre shader create error");
	ss->id = glCreateShader(ss->type);
	glerr("shader create error");
	
	glShaderSource(ss->id, VEC_LEN(&ss->strings), (const GLchar**)VEC_DATA(&ss->strings), NULL);
	glerr("shader source error");
	
	//printf("compiling\n");
	glCompileShader(ss->id);
	printLogOnFail(ss->id, ss);
	glerr("shader compile error");
}

ShaderProgram* loadCombinedProgram(char* path) {
	ShaderProgram* sp;
	ShaderSource* ss;
	char* spath;
	int bplen = strlen(SHADER_BASE_PATH);
	

	// grab the source
	spath = (char*)malloc(bplen + strlen(path) + 7);
	sprintf(spath, "%s/%s.glsl", SHADER_BASE_PATH, path);
	
	sp = makeShaderProgram();
	
	ss = loadShaderSource(spath);
	
// 	processIncludes(sp, ss);
	extractShaders(sp, ss);
	
	sp->id = glCreateProgram();
	
	int i;
	for(i = 0; i < 6; i++) {
		int j;
		ShaderSource* sss = sp->shaders[i];
		
		if(!sss) continue;
		
// 		for(j = 0; j < VEC_LEN(&sss->lines); j++)
// 		printf("%s:%d: '%s'\n", 
// 			VEC_ITEM(&sp->shaders[i]->lines, j).file_path,
// 			VEC_ITEM(&sp->shaders[i]->lines, j).file_line,
// 			VEC_ITEM(&sp->shaders[i]->lines, j).src
// 		);
		
		compileShader(sss);
		
		glAttachShader(sp->id, sss->id);
		glerr("Could not attach shader");

	}
	
	
	glLinkProgram(sp->id);
	printProgLogOnFail(sp->id);
	glerr("linking program");
	
	
	return sp;
}







