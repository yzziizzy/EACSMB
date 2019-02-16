#ifndef __EACSMB_common_math_h__
#define __EACSMB_common_math_h__



#include <stdio.h> 
#include <math.h> 
#include <stdint.h> 
#include <limits.h> 

#include "c3dlas/c3dlas.h" 
#include "c3dlas/meshgen.h"


// basic vertex formats for general use

typedef struct Vertex_PT {
	Vector p;
	struct { float u, v; } t;
} Vertex_PT;

typedef struct Vertex_PNT {
	Vector p, n;
	struct { float u, v; } t;
} Vertex_PNT;




static inline Vector4 color32ToVec4(uint32_t c) {
	Vector4 v;
	v.x = (c & 0x000000ff) / 255.0f;
	v.y = ((c & 0x0000ff00) >> 8) / 255.0f;
	v.z = ((c & 0x00ff0000) >> 16) / 255.0f;
	v.w = ((c & 0xff000000) >> 24) / 255.0f;
	return v;
}


#define COLOR_TO_VEC4(c) {\
	.x = ( (uint32_t)c & 0x000000ff) / 255.0f, \
	.y = (((uint32_t)c & 0x0000ff00) >> 8) / 255.0f, \
	.z = (((uint32_t)c & 0x00ff0000) >> 16) / 255.0f, \
	.w = (((uint32_t)c & 0xff000000) >> 24) / 255.0f, \
}


#endif // __EACSMB_common_math_h__
