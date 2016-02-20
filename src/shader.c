
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "utilities.h"
#include "shader.h"


const int BUFFER_SZ = 1024 * 8;
const char* SHADER_BASE_PATH = "./src/shaders/";



struct ShaderBuf {
	char** buffers;
	int allocSz;
	int count; 
	
};
 



void printLogOnFail(id) {
	
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
	fprintf(stderr, "Shader Log:\n%s", (char*)log);
	
	free(log);
}





GLuint loadShaderSource(char* source, int length, GLenum type) {
	
	GLuint id;
	glerr("pre shader create error");
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

int shaderPreProcess(char* path) { // TODO pass in some context
	
	struct ShaderBuf sb;
	FILE* f;
	char readBuf[4096];
	
	// an array of strings is fed to GL
	sb.buffers = calloc(1, sizeof(char*) * 64);
	sb.buffers[0] = malloc(sizeof(char) * BUFFER_SZ);
	sb.buffers[0][0] = 0;
	sb.allocSz = 64;
	sb.count = 1;
	
	
	f = fopen(path, "rb");
	if(!f) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		return NULL;
	}
	
	// append lines to the main buffers
	while(!feof(f)) {
		char* s;
		
		s = fgets(readBuf, 4096, f);
		
		if(s == NULL) { // eof or error
			if(ferror(f)) {
				fprintf(stderr, "Error reading file \"%s\"\n", path);
				return NULL;
			}
			
			//eof, readBuf unchanged
			return NULL; // BUG TODO ???
		}
		
		// scan the line for preprocessor directives
		
		// end of this shader, start of another -- ignore for now
		end = strstr(base, "#shader"); 
		end = strstr(base, "#include \"%s\""); 
		
		
		ws = strspn(readBuf, " \t");
		if(readBuf[ws] == '#') 
		
	}
	
	

	
}


struct sourceFile;

// a piece of a file, between others
struct sourceFragment {
	int srcLen; // if -1, it's a sourceFile struct
	char* src;
	
	char* basePath;
	char* filename;
	
	int srcStartingLine; // where the first line of this fragment is in the original source file
	int shaderStartingLine; // where the first line of this fragment falls in the final concatenated source
	int lineCount; // number of lines in this fragment
	GLuint shaderType;
	
	struct sourceFragment* next; 
	struct sourceFragment* prev; 
}
/* deprecated in favor of just a list of fragments
struct sourceFile {
	char* basePath;
	char* filename;
	int startingLine; // where the first line of this file falls in the final concatenated source
	int endingLine;
	
	struct sourceFragment* frags;
}
*/




struct sourceFragment* preloadFile(char* basePath, char* filename) {
	
	struct sourceFragment* sf, *tail;
	char* source, *base, *end;
	int srcLen;
	
	char includeName[256];
	
	sf = calloc(1, sizeof(struct sourceFragment));
	
	sf->basePath = basePath;
	sf->filename = filename;
	
	// TODO: basename shit
	
	source = readFile(filename, &srcLen);
	if(!sf->source) {
		free(spath); // TODO copypasta, fix this
		return NULL;
	}
	
	base = source;
	tail = sf;
	
	while(*base) {
		struct sourceFragment* frag;
		
		frag = nibble(&base);
		if(frag == NULL) break;
		
		frag->prev = tail;
		tail->next = frag;
		
		// nibble may return a list
		while(tail->next) tail = tail->next;
	}
	
	// walk the list and fill in line numbers
	
	return sf;
}

static struct sourceFrag* nibble(char** source) {
	
	struct sourceFragment* frag, *fileFrag;
	
	char* s;
	
	if(!**source) return NULL;
	
	frag = calloc(1, sizeof(struct sourceFrag));
	
	// walk over the source looking for directives
	s = strstr(*source, "\n#");
	if(s == NULL) { // we got to the end
		s = *source + strlen(*source);
		
		frag->srcLen = s - *source;
		frag->src = strndup(*source, frag->srcLen);
		
		*source = s;
		
		return frag;
	}
	
	// take the stuff in between *source and s
	
	frag->srcLen = s - *source + 1;
	frag->src = strndup(*source, frag->srcLen);
	
	s += 2; // skip the newline and pound
	
	if(0 == strncmp(s, "include", strlen("include"))) {
		*source = s; 
		
		fileFrag = nibbleFile(source);
		
		frag->next = fileFrag;
		fileFrag->prev = frag;
	}
	else if(0 == strncmp(s, "shader", strlen("shader"))) {
		// split it here, new shader
		
	}
	else {
		// unknown directive
		
	}
	
	// move to the next line
	s = strstr(s, '\n');
	if(s == NULL) *source = *source + strlen(*source);
	else *source = s + 1;
	
	return frag
}

static struct sourceFrag* nibbleFile(char** source) {
	struct sourceFragment* frag, *fileFrag;
	int cnt;
	
	frag = calloc(1, sizeof(struct sourceFragment));
	
	// extract the file name
	cnt = sscanf(*source + 9, " \"%255s\"", &includeName); // != 1 for failure
	if(cnt == EOF || cnt == 0) {
		return 2; // invalid parse 
	}
	
	frag->srcLen = -1;
	frag->sf = preloadFile(basePath, strdup(includeName))
	if(frag->sf == NULL) {
		
		
	}
	
	return frag;
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
	glerr("pre shader create 1");
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
	
	return prog;
}






