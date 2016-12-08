
#ifndef __EACSMB_UTILITIES_H__
#define __EACSMB_UTILITIES_H__

#define USE_KHR_DEBUG
#define NO_GL_GET_ERR_DEBUG

// i pronounce this one like "Grexit", greece's only smart move which they won't make cause they're greedy, short-sighted and dumb. just like the rest of us.
#define glexit(msg) _glexit(msg, __FILE__, __LINE__, __func__)

// returns NULL for no error, a human error string otherwise. the error is printed to stderr.
#define glerr(msg) _glerr(msg, __FILE__, __LINE__, __func__)


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



#define LIST(x) \
	struct x; \
	typedef struct x##List { \
		struct x* d; \
		struct x##List* next; \
	} x##List;

#define LISTU(x) \
	union x; \
	typedef struct x##List { \
		union x* d; \
		struct x##List* next; \
	} x##List;


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


// 128 bits to fill one slot of an aligned allocation for SSE 
struct array_info {
	uint32_t alloc_cnt;
	uint32_t next_index;
	
	uint64_t unused;
};


// get the array info struct
#define ar_info(ar) (&(((struct array_info*)(ar))[-1]))


#define ar_hasRoom(ar, n) (ar_info(ar)->next_index + n < ar_info(ar)->alloc_cnt)
#define ar_append(ar, x) do{ if(ar_hasRoom(ar)) { ar[ar_info(ar)->next_index++] = (x); } }while(0)
#define ar_append_direct(ar, x) do{ (ar)[ar_info(ar)->next_index++] = (x); }while(0)
#define ar_remove(ar)  do{ if(ar_info(ar)->next_index > 0) { ar_info(ar)->next_index--; } }while(0)
#define ar_tail(ar) (ar[ar_info(ar)->next_index])

#define ar_alloc(ar, cnt) (ar_alloc_internal((ar), sizeof(*(ar)), cnt)) 

void* ar_alloc_internal(void* ar, int sz, int cnt);












char* readFile(char* path, int* srcLen);



typedef struct VAOConfig {
	int sz;
	GLenum type;
	
} VAOConfig;

GLuint makeVAO(VAOConfig* details);


void initKHRDebug();

float fclamp(float val, float min, float max);
float fclampNorm(float val);

int iclamp(int val, int min, int max);
int iclampNorm(int val);

size_t strlnlen(const char* s);
char* strlndup(const char* s);
int   strlinecnt(const char* s);
char* pathJoin(const char* a, const char* b); 

#endif // __EACSMB_UTILITIES_H__
