
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <unistd.h>
#include <dirent.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>



#include "utilities.h"



// time code

double getCurrentTime() { // in seconds
	double now;
	struct timespec ts;
	static double offset = 0;
	
	// CLOCK_MONOTONIC_RAW is linux-specific.
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	
	now = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
	if(offset == 0) offset = now;
	
	return now - offset;
}

double timeSince(double past) {
	double now = getCurrentTime();
	return now - past;
}



// GPU timers

void query_queue_init(QueryQueue* q) {
	glGenQueries(6, q->qids);
	q->head = 0;
	q->used = 0;
}

void query_queue_start(QueryQueue* q) {
	if(q->used < 6) {
		glexit("");
		glBeginQuery(GL_TIME_ELAPSED, q->qids[q->head]);
		glexit("");
		q->head = (q->head + 1) % 6;
		q->used++;
	}
	else {
		fprintf(stderr, "query queue exhausted \n");
	}
}

void query_queue_stop(QueryQueue* q) {
	glEndQuery(GL_TIME_ELAPSED);
}

int query_queue_try_result(QueryQueue* q, uint64_t* time) {
	uint64_t p;
	int tail;
	
	if(q->used == 0) {
		return 2;
	}
	
	tail = (q->head - q->used + 6) % 6; 
	
	glGetQueryObjectui64v(q->qids[tail], GL_QUERY_RESULT_AVAILABLE, &p);
	if(GL_FALSE == p) {
		return 1; // the query isn't ready yet
	}
	
	glGetQueryObjectui64v(q->qids[tail], GL_QUERY_RESULT, time); 
	q->used--;
	
	return 0;
}

int tryQueryTimer(GLuint id, uint64_t* time) {
	uint64_t p;
	
	glGetQueryObjectui64v(id, GL_QUERY_RESULT_AVAILABLE, &p);
	if(GL_TRUE == p) { 
		glGetQueryObjectui64v(id, GL_QUERY_RESULT, time); 
		return 0;
	}
	
	return 1;
}




static uint32_t charToVal(char c) {
	switch(c) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
	};
	return 0;
}

static uint32_t doubleChar(char c) {
	uint32_t v = charToVal(c);
	return v & v << 4;
}



/*
Parse color strings:
#rrggbb
#rrggbbaa
#rgb
#rgba
green
*/
uint32_t parseColor(char* s) {
	uint32_t c = 0x000000ff;
	
	if(!s) return 0xffffffff;
	
	if(s[0] == '#') {
		int len = strlen(s+1);
		
		if(len == 3 || len == 4) {
			c &= (doubleChar(s[1]) << 24) & (doubleChar(s[2]) << 16) & (doubleChar(s[3]) << 8);
			
			if(len == 4) {
				c &= doubleChar(s[4]);
			}
		}
		else if(len == 6 || len == 8) {
			c &= (charToVal(s[1]) << 28) &
				(charToVal(s[2]) << 24) &
				(charToVal(s[3]) << 20) &
				(charToVal(s[4]) << 16) &
				(charToVal(s[5]) << 12) &
				(charToVal(s[6]) << 8);
				
			if(len == 8) {
				c &= (charToVal(s[7]) << 4) & charToVal(s[8]);
			}
		}
		
		return c;
	}
	
	// check color strings. 
	// TODO: some clever lookup
	if(0 == strcasecmp(s, "red")) return 0xff0000ff;
	if(0 == strcasecmp(s, "green")) return 0xff00ff00;
	if(0 == strcasecmp(s, "blue")) return 0xffff0000;
	if(0 == strcasecmp(s, "magenta")) return 0xffff00ff;
	if(0 == strcasecmp(s, "yellow")) return 0xff00ffff;
	if(0 == strcasecmp(s, "cyan")) return 0xffffff00;
	if(0 == strcasecmp(s, "black")) return 0xff000000;
	if(0 == strcasecmp(s, "white")) return 0xffffffff;
	if(0 == strcasecmp(s, "gray")) return 0xff888888;
	if(0 == strcasecmp(s, "silver")) return 0xffbbbbbb;
	if(0 == strcasecmp(s, "darkgray")) return 0xff444444;
	if(0 == strcasecmp(s, "darkred")) return 0xff000088;
	if(0 == strcasecmp(s, "darkgreen")) return 0xff008800;
	if(0 == strcasecmp(s, "darkblue")) return 0xff880000;
	if(0 == strcasecmp(s, "navy")) return 0xff440000;
	if(0 == strcasecmp(s, "forest")) return 0xff004400;
	if(0 == strcasecmp(s, "maroon")) return 0xff000044;
	if(0 == strcasecmp(s, "darkyellow")) return 0xff008888;
	if(0 == strcasecmp(s, "olive")) return 0xff004444;
	
	return 0xffffffff;
}





// TODO BUG: fix prepending a \n everywhere
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



char* readFileRaw(char* path, int* srcLen) {
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
	
	contents = (char*)malloc(fsize + 1);
	
	fread(contents, sizeof(char), fsize, f);
	contents[fsize] = 0;
	
	fclose(f);
	
	if(srcLen) *srcLen = fsize + 1;
	
	return contents;
}




// convenience
void texParams2D(GLenum type, GLenum filter, GLenum wrap) {
	glTexParameterf(type, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameterf(type, GL_TEXTURE_MAG_FILTER, filter == GL_NEAREST ? GL_NEAREST : GL_LINEAR);
	glTexParameterf(type, GL_TEXTURE_WRAP_S, wrap);
	glTexParameterf(type, GL_TEXTURE_WRAP_T, wrap);
	glexit("");
}

// returns 1 if the tex was created
int glGenBindTexture(GLuint* tex, GLenum type) {
	int new = 0;
	if(!*tex) {
		glGenTextures(1, tex);
		new = 1;
	}
	glBindTexture(GL_TEXTURE_2D_ARRAY, *tex);
	
	return new;
}


static int attrib_type_size(GLenum t) {
	switch(t) {
		case GL_DOUBLE: 
		case GL_UNSIGNED_INT64_ARB:
			return 8; 
		case GL_FLOAT: 
		case GL_INT: 
		case GL_UNSIGNED_INT: 
			return 4; 
		case GL_SHORT: 
		case GL_UNSIGNED_SHORT: 
			return 2; 
		case GL_BYTE: 
		case GL_UNSIGNED_BYTE: 
			return 1; 
		case GL_MATRIX_EXT: // abused for this function. does not conflict with anything
			return 4*16; 
		default:
			fprintf(stderr, "Unsupported VAO type\n");
			int a = *((int*)0);
			exit(2);
	}	
}



GLuint makeVAO(VAOConfig* details) {
	int i; // packed data is expected
	uintptr_t offset = 0;
	int stride = 0;
	int attrSlot = 0;
	GLuint vao;
	
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	for(i = 0; details[i].sz != 0; i++) {
		stride += details[i].sz * attrib_type_size(details[i].type);
	}
	
	for(i = 0; details[i].sz != 0; i++) {
		GLenum t;
		int ds;
		
		glEnableVertexAttribArray(i);
		t = details[i].type;
		if(t == GL_FLOAT) { // works only for my usage
			glVertexAttribFormat(attrSlot, details[i].sz, t, GL_FALSE, (GLuint)offset);
		}
		else if(t == GL_MATRIX_EXT) {
			glVertexAttribFormat(attrSlot++, 4, GL_FLOAT, GL_FALSE, (GLuint)offset);
			glVertexAttribFormat(attrSlot++, 4, GL_FLOAT, GL_FALSE, (GLuint)offset+4*4);
			glVertexAttribFormat(attrSlot++, 4, GL_FLOAT, GL_FALSE, (GLuint)offset+4*8);
			glVertexAttribFormat(attrSlot  , 4, GL_FLOAT, GL_FALSE, (GLuint)offset+4*12);
		}
		else if(t == GL_UNSIGNED_INT64_ARB) {
			glVertexAttribLFormat(i, details[i].sz, t, (GLuint)offset);
		}
		else {
			glVertexAttribIFormat(i, details[i].sz, t, (GLuint)offset);
		}
		glerr("vao init");
		
		if(t == GL_UNSIGNED_BYTE || t == GL_BYTE) ds = 1;
		else if(t == GL_UNSIGNED_SHORT || t == GL_SHORT) ds = 2;
		else if(t == GL_UNSIGNED_INT64_ARB) ds = 8;
		else ds = 4;
		
		offset += ds * details[i].sz;
		attrSlot++;
	}
	glexit("vao init");
	
	return vao;
}


size_t calcVAOStride(int bufferIndex, VAOConfig* details) {
	int i;
	int stride = 0;
	
	// determine the buffer's range
	for(i = 0; details[i].sz != 0; i++) {
		if(details[i].bufferIndex == bufferIndex) {
			stride += details[i].sz * attrib_type_size(details[i].type);
		}
	}
	
	return stride;
}


// returns stride
size_t updateVAO(int bufferIndex, VAOConfig* details) {
	
	int i;
	int startIndex = -1, endIndex = -1;
	int stride = 0;
	
	// determine the buffer's range
	for(i = 0; details[i].sz != 0 && endIndex == -1; i++) {
		if(startIndex == -1) {
			if(details[i].bufferIndex == bufferIndex) startIndex = i;
		}
		else {
			
			if(details[i].bufferIndex != bufferIndex) {
				endIndex = i - 1;
				
			}
		}
		
		if(startIndex != -1 && endIndex == -1) {
			stride += details[i].sz * attrib_type_size(details[i].type);
		}		
	}
	if(endIndex == -1) endIndex = i - 1;
	
	
	size_t offset = 0;
	int attrSlot = startIndex;
	for(i = startIndex; i <= endIndex; i++) {
		glEnableVertexAttribArray(attrSlot);
		
		GLenum t = details[i].type;
		
		if(t == GL_FLOAT || details[i].normalized == GL_TRUE) { // works only for my usage
			glVertexAttribPointer(attrSlot, details[i].sz, t, details[i].normalized, stride, (GLvoid*)offset);
			glVertexAttribDivisor(attrSlot, details[i].divisor);
			glexit("");
		}
		else if(t == GL_MATRIX_EXT) {
			
			glEnableVertexAttribArray(attrSlot+1);
			glEnableVertexAttribArray(attrSlot+2);
			glEnableVertexAttribArray(attrSlot+3);
			
			glVertexAttribPointer(attrSlot,   4, GL_FLOAT, details[i].normalized, stride, (GLvoid*)offset);
			glVertexAttribPointer(attrSlot+1, 4, GL_FLOAT, details[i].normalized, stride, (GLvoid*)offset+4*4);
			glVertexAttribPointer(attrSlot+2, 4, GL_FLOAT, details[i].normalized, stride, (GLvoid*)offset+4*8);
			glVertexAttribPointer(attrSlot+3, 4, GL_FLOAT, details[i].normalized, stride, (GLvoid*)offset+4*12);

			glVertexAttribDivisor(attrSlot,   details[i].divisor);
			glVertexAttribDivisor(attrSlot+1, details[i].divisor);
			glVertexAttribDivisor(attrSlot+2, details[i].divisor);
			glVertexAttribDivisor(attrSlot+3, details[i].divisor);
			glexit("");
			
			attrSlot += 3;
		}
		else if(t == GL_UNSIGNED_INT64_ARB) {
// 			glEnableVertexAttribArray(attrSlot+1);
			glVertexAttribLPointer(attrSlot, details[i].sz, GL_UNSIGNED_INT64_ARB, stride, (GLvoid*)offset);
			glVertexAttribDivisor(attrSlot, details[i].divisor);
// 			glVertexAttribDivisor(attrSlot+1, details[i].divisor);
// 			attrSlot += 1;
		}
		else {
			glVertexAttribIPointer(attrSlot, details[i].sz, t, stride, (GLvoid*)offset);
			glVertexAttribDivisor(attrSlot, details[i].divisor);
			glexit("");
		}
		
		
		int ds = 0;
		if(t == GL_UNSIGNED_BYTE || t == GL_BYTE) ds = 1;
		else if(t == GL_UNSIGNED_SHORT || t == GL_SHORT) ds = 2;
		else if(t == GL_MATRIX_EXT) ds = 4*16;
		else if(t == GL_UNSIGNED_INT64_ARB) ds = 8;
		else ds = 4;
		
		offset += ds * details[i].sz;
		attrSlot++;
	} 
	

	glexit("vao update");
	return stride;
}



// length of the line, or length of the string if no \n found
size_t strlnlen(const char* s) {
	char* n;
	
	n = strchr(s, '\n');
	if(!n) return strlen(s);
	
	return n - s;
}

// strdup a line
char* strlndup(const char* s) {
	return strndup(s, strlnlen(s));
}

// line count;
int strlinecnt(const char* s) {
	int n;

	if(!*s) return 0;
	
	n = 1;
	while(*s) // just to make you cringe
		if(*s++ == '\n') 
			n++;
	
	return n;
}

char* pathJoin(const char* a, const char* b) {
	int alen, blen;
	char* o;
	
	
	alen = a ? strlen(a) : 0;
	blen = b ? strlen(b) : 0;
	
	o = malloc(alen + blen + 2);
	
	strcpy(o, a ? a : "");
	o[alen] = '/'; // TODO: fix the concat here
	strcpy(o + alen + 1, b ? b : "");
	o[alen + blen + 1] = 0; 
	
	return o;
}


// gets a pointer to the first character of the file extension, or to the null terminator if none
char* pathExt(char* path) {
	int i;
	int len = strlen(path);
	
	for(i = len - 1; i >= 0; i--) {
		char c = path[i];
		if(c == '.') return path + i;
		else if(c == '/') break;
	} 
	
	return path + len;
}

// gets a pointer to the first character of the file extension, or to the null terminator if none
// also provides the length of the path without the period and extension
char* pathExt2(char* path, int* end) {
	int i;
	int len = strlen(path);
	
	for(i = len - 1; i >= 0; i--) {
		char c = path[i];
		if(c == '.') {
			if(end) *end = i > 0 ? i : 0; 
			return path + i + 1;
		}
		else if(c == '/') break;
	} 
	
	if(end) *end = len;
	return path + len;
}
