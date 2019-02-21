
#ifndef __EACSMB_UTILITIES_H__
#define __EACSMB_UTILITIES_H__

#include <stdio.h> // fprintf
#include <stdlib.h> // exit
#include <strings.h> // strcasecmp. yes, it's "strings" with an s at the end.

#include "common_gl.h"



#define MAX(a,b) ({ \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a > _b ? _a : _b; \
})
#define MIN(a,b) ({ \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a < _b ? _a : _b; \
})
#define MAXE(a,b) ({ \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a >= _b ? _a : _b; \
})
#define MINE(a,b) ({ \
	__typeof__ (a) _a = (a); \
	__typeof__ (b) _b = (b); \
	_a <= _b ? _a : _b; \
})


#define CHECK_OOM(p) \
if(!(p)) { \
	fprintf(stderr, "OOM for %s at %s:%d. Buy more ram\n", #p, __FILE__, __LINE__); \
	exit(2); \
}

#define GOTO_OOM(p, label) \
if(!(p)) { \
	fprintf(stderr, "OOM for %s at %s:%d. Buy more ram\n", #p, __FILE__, __LINE__); \
	goto label; \
}


#define pcalloc(x) x = calloc(1, sizeof(*(x)))

#ifndef NO_TERM_COLORS
	#define TERM_COLOR_BLACK   "\x1b[30m"
	#define TERM_COLOR_RED     "\x1b[31m"
	#define TERM_COLOR_GREEN   "\x1b[32m"
	#define TERM_COLOR_YELLOW  "\x1b[33m"
	#define TERM_COLOR_BLUE    "\x1b[34m"
	#define TERM_COLOR_MAGENTA "\x1b[35m"
	#define TERM_COLOR_CYAN    "\x1b[36m"
	#define TERM_COLOR_GRAY    "\x1b[37m"
	#define TERM_RESET         "\x1b[0m"
	#define TERM_BOLD          "\x1b[1m"
	#define TERM_NORMAL        "\x1b[22m"
	#define TERM_BK_BLACK      "\x1b[40m"
	#define TERM_BK_RED        "\x1b[41m"
	#define TERM_BK_GREEN      "\x1b[42m"
	#define TERM_BK_YELLOW     "\x1b[43m"
	#define TERM_BK_BLUE       "\x1b[44m"
	#define TERM_BK_MAGENTA    "\x1b[45m"
	#define TERM_BK_CYAN       "\x1b[46m"
	#define TERM_BK_GRAY       "\x1b[47m"
#else
#endif


	
// super nifty site:
// http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static inline int nextPOT(int in) {
	
	in--;
	in |= in >> 1;
	in |= in >> 2;
	in |= in >> 4;
	in |= in >> 8;
	in |= in >> 16;
	in++;
	
	return in;
}


#include "ds.h"



typedef void (*progess_fn_t)(float*); 


// cpu clock stuff
double getCurrentTime();
double timeSince(double past);


// gpu timer queries
typedef struct QueryQueue {
	GLuint qids[6];
	int head, used;
} QueryQueue;

void query_queue_init(QueryQueue* q);
void query_queue_start(QueryQueue* q);
void query_queue_stop(QueryQueue* q);
int query_queue_try_result(QueryQueue* q, uint64_t* time);


uint32_t parseColor(char* s);



// TODO BUG: fix prepending a \n everywhere
char* readFile(char* path, int* srcLen);
char* readFileRaw(char* path, int* srcLen);

int glGenBindTexture(GLuint* tex, GLenum type);
void texParams2D(GLenum type, GLenum filter, GLenum wrap);


typedef struct VAOConfig {
	int bufferIndex; // for instancing
	
	int sz;
	GLenum type;
	
	int divisor;
	
	int normalized;
	
} VAOConfig;

GLuint makeVAO(VAOConfig* details);
size_t updateVAO(int bufferIndex, VAOConfig* details); 
size_t calcVAOStride(int bufferIndex, VAOConfig* details);



#define streq(a, b) (0 == strcmp(a, b))
#define strcaseeq(a, b) (0 == strcasecmp(a, b))

size_t strlnlen(const char* s);
char* strlndup(const char* s);
int   strlinecnt(const char* s);
char* pathJoin(const char* a, const char* b); 

// gets a pointer to the first character of the file extension, or to the null terminator if none
char* pathExt(char* path);

// gets a pointer to the first character of the file extension, or to the null terminator if none
// also provides the length of the path without the period and extension
char* pathExt2(char* path, int* end);




static char* strcatdup2(char* a, char* b) {
	size_t la = strlen(a);
	size_t lb = strlen(b);
	char* o = malloc((la + lb + 1) * sizeof(char));
	strcpy(o, a);
	strcpy(o + la, b);
	return o;
}

static char* strcatdup3(char* a, char* b, char* c) {
	size_t la = strlen(a);
	size_t lb = strlen(b);
	size_t lc = strlen(c);
	char* o = malloc((la + lb + lc + 1) * sizeof(char));
	strcpy(o, a);
	strcpy(o + la, b);
	strcpy(o + la + lb, c);
	return o;
}

static char* strcatdup4(char* a, char* b, char* c, char* d) {
	size_t la = strlen(a);
	size_t lb = strlen(b);
	size_t lc = strlen(c);
	size_t ld = strlen(d);
	char* o = malloc((la + lb + lc + ld + 1) * sizeof(char));
	strcpy(o, a);
	strcpy(o + la, b);
	strcpy(o + la + lb, c);
	strcpy(o + la + lb + lc, d);
	return o;
}



// these versions return the length
static char* strcatdup2_len(const char* a, const char* b, size_t* len) {
	size_t la = strlen(a);
	size_t lb = strlen(b);
	char* o = malloc((la + lb + 1) * sizeof(char));
	strcpy(o, a);
	strcpy(o + la, b);
	if(len) *len = la + lb;
	return o;
}

static char* strcatdup3_len(const char* a, const char* b, const char* c, size_t* len) {
	size_t la = strlen(a);
	size_t lb = strlen(b);
	size_t lc = strlen(c);
	char* o = malloc((la + lb + lc + 1) * sizeof(char));
	strcpy(o, a);
	strcpy(o + la, b);
	strcpy(o + la + lb, c);
	if(len) *len = la + lb + lc;
	return o;
}

static char* strcatdup4_len(const char* a, const char* b, const char* c, const char* d, size_t* len) {
	size_t la = strlen(a);
	size_t lb = strlen(b);
	size_t lc = strlen(c);
	size_t ld = strlen(d);
	char* o = malloc((la + lb + lc + ld + 1) * sizeof(char));
	strcpy(o, a);
	strcpy(o + la, b);
	strcpy(o + la + lb, c);
	strcpy(o + la + lb + lc, d);
	if(len) *len = la + lb + lc + ld;
	return o;
}








#endif // __EACSMB_UTILITIES_H__
