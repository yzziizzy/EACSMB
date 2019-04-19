

// This file is automatically included in every translation unit by the build system


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <assert.h>


#ifdef NDEBUG
	#define DebugBreak() ((void)0)
#else
	// requires the compiler to use AT&T assembly
	#define DebugBreak() __asm__("int %3")
#endif

#define clz(x) __builtin_clz(x)
#define ctz(x) __builtin_ctz(x)
