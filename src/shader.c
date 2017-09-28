
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

void printProgLogOnFail(id) {
	
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

GLenum indexToEnum(int index) {
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


int nameToIndex(char* name) {
	
	if(0 == strcasecmp(name, "VERTEX")) return 0;
	if(0 == strcasecmp(name, "TESS_CONTROL")) return 1;
	if(0 == strcasecmp(name, "TESS_EVALUATION")) return 2;
	if(0 == strcasecmp(name, "GEOMETRY")) return 3;
	if(0 == strcasecmp(name, "FRAGMENT")) return 4;
	if(0 == strcasecmp(name, "COMPUTE")) return 5;
	
	return -1;
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


/**** newer shit ****/



typedef VEC(char*) stringlist;

typedef struct {
	char* src;
	char* file_path;
	int file_line;
} LineInfo;

typedef struct {
	VEC(LineInfo) lines;
	VEC(char*) strings; // only used in final separated shaders
	char* path;
	
	uint32_t version;
	GLuint id;
	
} NewShaderSource;

typedef struct {
	HashTable(NewShaderSource*) sources;
	
	char* name;
	
	NewShaderSource* shaders[6];
	
	GLuint id;
	
} NewShaderProgram;


// removes \n chars
void strsplit(char* source, stringlist* out) { 
	char* s = source;
	char* lstart = source;
	
	while(*s) {
		if(*s == '\n') {
			VEC_PUSH(out, strndup(lstart, s - lstart));
			lstart = s + 1;
		}
		s++;
	}
	
	// handle the last line
	if(s > lstart) {
		VEC_PUSH(out, strndup(lstart, s - lstart));
	}
}

NewShaderSource* makeShaderSource() {
	NewShaderSource* n;
	
	n = calloc(1, sizeof(*n));
	CHECK_OOM(n);
	
	VEC_INIT(&n->lines);
	VEC_INIT(&n->strings);
}

NewShaderSource* loadNewShaderSource(char* path) {
	NewShaderSource* ss;
	stringlist l;
	int i;
	char* source;

	
	source = readFile(path, NULL);
	if(!source) {
		// TODO copypasta, fix this
		fprintf(stderr, "failed to load shader file '%s'\n", path);
		return NULL;
	}
	
	VEC_INIT(&l);
	ss = makeShaderSource();
	
	ss->path = path;
	strsplit(source, &l);
	
	for(i = 0; i < VEC_LEN(&l); i++) {
		LineInfo* li;
		VEC_INC(&ss->lines);
		
		li = &VEC_ITEM(&ss->lines, i);
		li->src = VEC_ITEM(&l, i);
		li->file_path = path;
		li->file_line = i + 1;
	}
	
	VEC_FREE(&l);
	
	return ss;
}


char* extractFileName(char* src) {
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

char* realFromSiblingPath(char* sibling, char* file) {
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

void processIncludes(NewShaderProgram* sp, NewShaderSource* ss) {
	int i;
	char* s, *includeFileName, *includeFilePath;
	
	for(i = 0; i < VEC_LEN(&ss->lines); i++) {
		NewShaderSource* iss;
		LineInfo* li = &VEC_ITEM(&ss->lines, i);
		
		
		if(0 == strncmp("#include", li->src, strlen("#include"))) {
			s = li->src + strlen("#include");
			
			includeFileName = extractFileName(s);
			includeFilePath = realFromSiblingPath(ss->path, includeFileName);
			
			// TODO: recursion detection
			iss = loadNewShaderSource(includeFilePath);
			
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


void extractShaders(NewShaderProgram* sp, NewShaderSource* raw) {
	int i;
	char* s;
	char typeName[24];
	int commonVersion = 0;
	
	NewShaderSource* curShader = NULL;
	
	/*
	NewShaderSource* common;
	common = makeShaderSource();
	*/
	
	for(i = 0; i < VEC_LEN(&raw->lines); i++) {
		int cnt, index;
		int version;
		LineInfo* li = &VEC_ITEM(&raw->lines, i);
		
		
		// extract the GLSL version to be prepended to the shader
		if(0 == strncmp("#version", li->src, strlen("#version"))) {
			s = li->src + strlen("#version");
			
			sscanf(s, " %d", &version);
			
			if(!curShader) {
				commonVersion = version;
			}
			else {
				curShader->version = version;
			}
			
			// comment out this line
			li->src[0] = '/';
			li->src[1] = '/';
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
			
			if(commonVersion) curShader->version = commonVersion;
			
			// make room for the version directive, added later
			VEC_INC(&curShader->lines);
			VEC_INC(&curShader->strings);
			
			// comment out this line
			li->src[0] = '/';
			li->src[1] = '/';
		}
		else {
			if(curShader) {
				// copy line 
				VEC_PUSH(&curShader->lines, *li);
				VEC_PUSH(&curShader->strings, li->src);
			}
		}
		
	}
	
	
	// fill in version info
	for(i = 0; i < 6; i++) {
		NewShaderSource* ss = sp->shaders[i];
		char* s;
		
		if(!ss) continue;
		
		s = malloc(20);
		sprintf(s, "#version %0.3u", ss->version);
		VEC_ITEM(&ss->strings, 0) = s;
		VEC_ITEM(&ss->lines, 0).src = s;
		VEC_ITEM(&ss->lines, 0).file_path = NULL;
		VEC_ITEM(&ss->lines, 0).file_line = -1;
	}
}



NewShaderProgram* makeShaderProgram() {
	NewShaderProgram* sp;
	
	sp = calloc(1, sizeof(*sp));
	CHECK_OOM(sp);
	
	HT_init(&sp->sources, 0);
	
	return sp;
}


NewShaderProgram* loadNewProgram(char* path) {
	NewShaderProgram* sp;
	NewShaderSource* ss;
	
	sp = makeShaderProgram();
	
	ss = loadNewShaderSource(path);
	
	processIncludes(sp, ss);
	extractShaders(sp, ss);
	
	int i;
	for(i = 0; i < VEC_LEN(&sp->shaders[0]->lines); i++) {
		printf("%s:%d: '%s'\n", 
			VEC_ITEM(&sp->shaders[0]->lines, i).file_path,
			VEC_ITEM(&sp->shaders[0]->lines, i).file_line,
			VEC_ITEM(&sp->shaders[0]->lines, i).src
		);
	}
	
	
	return sp;
}


void testsplit(char* source) {
	int i;
	NewShaderSource* ss;
	
	loadNewProgram(source);
// 	ss = loadNewShaderSource(source);
	
	

	
}


/*^^^ newer shit ^^^*/


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
	
	
	cnt = sscanf(base + 8, " %23s", typeName); // != 1 for failure
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



struct sourceFile;

// a piece of a file, between others
struct sourceFragment {
	int srcLen; // if -1, it's a sourceFile struct
	char* src;
	
	//char* basePath;
	char* filename;
	
	int srcStartingLine; // where the first line of this fragment is in the original source file
	int shaderStartingLine; // where the first line of this fragment falls in the final concatenated source
	int lineCount; // number of lines in this fragment
	int shaderType;
	
	struct sourceFragment* next; 
	struct sourceFragment* prev; 
};
/* deprecated in favor of just a list of fragments
struct sourceFile {
	char* basePath;
	char* filename;
	int startingLine; // where the first line of this file falls in the final concatenated source
	int endingLine;
	
	struct sourceFragment* frags;
}
*/

int scanShaderType(const char* s) {
	int cnt, type;
	
	char typeName[34];
		
	cnt = sscanf(s, " %23s", typeName); // != 1 for failure
	if(cnt == EOF || cnt == 0) {
		printf("unrecognized shader type \n"); 
		return -1;
	}
	
	printf(" %s", typeName); 
	
	type = nameToIndex(typeName);
	if(type == -1) {
		fprintf(stderr, "Invalid shader type %s\n", typeName);
		return -1;
	}	
	
	return type;
}

int newTopLevel(char* file) {
	
	struct sourceFragment* sf;
	
	sf = preloadShader(".", file);
	
	//preloadShader(sf);
	
}


static struct sourceFragment* nibble(char** source);
static struct sourceFragment* nibbleFile(char** source, char* basePath);

struct sourceFragment* preloadFile(char* basePath, char* filename) {
	struct sourceFragment* frag;
	struct sourceFragment* head, *tail;
	char* source, *base, *end, *s;
	int srcLen;
	
	char typeName[24];
	
	char* truebase;
	char* fullpath;
	
	fullpath = pathJoin(basePath, filename);
	printf("preloading file '%s'\n", fullpath);
	
	// TODO: basename shit
	
	source = readFile(fullpath, &srcLen);
	if(!source) {
		// TODO copypasta, fix this
		return NULL;
	}
	
// 	sf = calloc(1, sizeof(struct sourceFragment));
// 	
// 	sf->basePath = basePath;
// 	sf->filename = filename;
	

	
	truebase = strdup(dirname(fullpath));
	
	s = source;
	base = source;
	//tail = sf;
	
	struct sourceFragment fake;
	fake.next = NULL;
	fake.prev = NULL;
	
	tail = &fake;
	head = &fake;
	
	int i = 0;
	
	while(*s && i++ < 100) {
		
		printf("~~~~~~~~~~~~~~~\n");
		s = strstr(s, "\n#");
		if(!s) {
			printf("nothing left\n");
			break;
		}
	
		s += 2;
		
		printf("%.10s\n", s);
		if(0 == strncmp(s, "include", strlen("include"))) {
			
			printf("nibbling file\n\n");
			
			// nibble the part before this
			frag = calloc(1, sizeof(struct sourceFragment));
			tail->next = frag;
			frag->prev = tail;
			frag->shaderType = -1;
			
			frag->srcLen =  s - base - 2;
			frag->src = strndup(base, frag->srcLen);
			frag->filename = fullpath;
			
			tail = frag;
			
			frag = nibbleFile(&s, truebase);
			if(!frag) {
				printf("preload file failed\n");
				return NULL; // TODO: memory leak
			}
			
			if(!head) head = frag;
			
			if(tail) { 
				tail->next = frag;
				frag->prev = tail;
			}
			else {
				tail = frag;
			}
			
			// advance tail to the end. preloadFile can return a list.
			while(tail->next) tail = tail->next;
			
			// bove the base pointer up
			base = s;
		}
		else if(0 == strncmp(s, "shader", strlen("shader"))) {
			// keep going for now
			//base = s;
			
			// nibble the part before this
			frag = calloc(1, sizeof(struct sourceFragment));
			tail->next = frag;
			frag->prev = tail;
			frag->shaderType = -1;
			
			frag->srcLen =  s - base - 2;
			printf("before part len = %d\n", frag->srcLen);
			frag->src = strndup(base, frag->srcLen);
			printf("duipped len = %d\n", strlen(frag->src));
			frag->filename = fullpath;
			
			tail = frag;
			
			// add the shader fragment
			frag = calloc(1, sizeof(struct sourceFragment));
			tail->next = frag;
			frag->prev = tail;
			
			frag->src = strlndup(s - 1);
			printf("after duipped len = %d\n", strlen(frag->src));
			frag->srcLen =  strlen(frag->src);
			frag->filename = fullpath;
			
			tail = frag;
			
			// skip to end of line
			base = strchr(s, '\n');
			
			frag->shaderType = scanShaderType(s + 6);
			
			printf("nibbling shader\n");
		} 
		else { // some other directive
			// keep going
			//base = s;
			printf("nibbling else\n");
		}
	}
	

	printf("%d %d\n", base, s);
	int n = strlen(base);
	if(n) {
		printf("extra left\n\n");
		// add the fragment 
		frag = calloc(1, sizeof(struct sourceFragment));
		tail->next = frag;
		frag->prev = tail;
		frag->shaderType = -1;
		
		frag->srcLen =  n;
		frag->src = strndup(base, frag->srcLen);
		frag->filename = fullpath;
	} 
	else {
		
		printf("no extra left\n\n");
	}
	
	// walk the list and fill in line numbers
	
	return head->next;
}


static struct sourceFragment* nibbleFile(char** source, char* basePath) {
	struct sourceFragment* frag, *fileFrag;
	int cnt;
	
	char includeName[256];
	printf("%.10s\n", (*source)+7);
	// extract the file name
	
	char* s = *source + 7;
	
	//cnt = sscanf((*source) + 7, " \"%255s\"", includeName); // != 1 for failure
	
	char* a = strchr(s, '"');
	char* b = strchr(a + 1, '"');
	char* c = strndup(a + 1, b - a - 1);
	
	printf("duped string '%s'\n", c);
	
	
	//if(cnt == EOF || cnt == 0) {
	//	fprintf(stderr, "Could not parse included file name\n");
	//	return NULL; // invalid parse 
	//}
	
	printf("including file %s\n", c); 
	// TODO: basename magic here
	//return NULL;
	return preloadFile(basePath, c);
}


Shader* preloadShader(char* basePath, char* filename) {
	struct sourceFragment* frag, *x, *y;
	int lines;
	
	struct sourceFragment* sfrags[6] = {0, 0, 0, 0, 0, 0};
	short slen[6] = {0, 0, 0, 0, 0, 0};
	char** schars[6] = {0, 0, 0, 0, 0, 0};
	
	frag = preloadFile(basePath, filename);
	if(!frag) return NULL;
	
	// fill in line numbers
	lines = 0;
	x = frag;
	while(x) {
		int n;
		
		n = strlinecnt(x->src);
		
		x->srcStartingLine = lines;
		x->lineCount = n;
		
		lines += n;
		
		x = x->next;
	}
	
	// extract shaders
	
	int prevType = -1;
	x = frag;
	while(x) {
		int n;
		printf("looping prevtype=%d n=%d x->sT=%d \n", prevType, n, x->shaderType);
		printf("  %.5s\n", x->src);
		if(x->shaderType == prevType || x->shaderType < 0) {
			if(prevType != -1) slen[prevType]++;
			x = x->next;
			continue;
		}
		
		
		
		// break the chain here

		printf("^^ breaking chain\n");
		// TODO: null deref checks
		x->prev->next = NULL;
		x->prev = NULL;
		
		
		// save the head
		// should filter out the "shader" ones
		if(sfrags[x->shaderType]) {
			fprintf(stderr, "shader already found %d\n", x->shaderType);
			
			return NULL; // TODO: error handling
		}
		
		sfrags[x->shaderType] = x;
		prevType = x->shaderType;
		
		// keep count along the way
		slen[x->shaderType]++;
		
		x = x->next;
	}
	
	int i, n, z;
	// make char* arrays for opengl
	for(i = 0; i < 6; i ++) {
		printf(" i = %d \n", i);
		printf(" slen = %d \n", slen[i]);
		// allocate an array of char*'s twice as big as it needs to be
		// the extra pointer is for the #line directives 
		schars[i] = malloc(slen[i] * sizeof(char*) * 2);
		
		x = sfrags[i];
		n = 0;
		int linecnt = 1;
		while(x) {
			printf("%d, %.20s\n", n, x->src);
			// TODO: prepend the line strings
			
			z = snprintf(NULL, 0, "#line %d %d \n", linecnt, i+1);
			printf("z = %d\n", z);
			schars[i][n] = malloc(z * sizeof(char)+1);
			snprintf(schars[i][n], z+1, "#line %d %d \n", linecnt, i+1);
			printf("#line %d %d\n", linecnt, i+1);
			
			n++;
			
			if(0 != strncmp(x->src, "#shader", strlen("#shader"))) 
				schars[i][n++] = x->src;
			
			linecnt += x->lineCount;
			
			
			x = x->next;
		}
		
		
	}
	
	
	// print out the strings as debug info
	for(i = 0; i < 6; i ++) {
		
		if(slen[i] == 0) continue;
		
		printf("-%d-----------------------------------------", i);
// 		type = nameToEnum(typeName);
// 		if(type == -1) {
// 			fprintf(stderr, "Invalid shader type %s\n", typeName);
// 			return 3;
// 		}

		x = sfrags[i];
		n = 0;
		int linecnt = 1;
		if(i == 0) {
		while(x) {
			printf("========\n");
			printf(x->src);
			
			x = x->next;
		}
		}
		
		GLuint id;
		glerr("pre shader create error");
		id = glCreateShader(indexToEnum(i));
		glerr("shader create error");
		
		glShaderSource(id, slen[i], schars[i], NULL);
		glerr("shader source error");
		
		printf("compiling\n");
		glCompileShader(id);
		printLogOnFail(id);
		glerr("shader compile error");
		
		//	glAttachShader(0, id);
	glerr("Could not attach shader");

			
		
	//	id = loadShaderSource(base, end - base, type);
	//	if(!id) { // BUG: look up the real failure code
	//		return 4;
	//	}
		
		//schars[i] = malloc(slen[i] * sizeof(char*));
		
		//printf("%d, %.20s\n", i, schars[i]);
		
		
	}
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
	
	// HACK
	testsplit(spath);
	exit(1);
	
	//parse out the contents one shader at a time
	
	base = source;
	int c = 0;
	
	printf("Loading %s: (program %d)\n   ", spath, prog->id);
	while(!extractShader(&base, prog->id)) glexit("");
	printf("\n");
	
	free(source);
	free(spath);
	
	glLinkProgram(prog->id);
	printProgLogOnFail(prog->id);
	glerr("linking program");
	
	
	return prog;
}






