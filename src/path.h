#ifndef __EACSMB__path_h__
#define __EACSMB__path_h__


#include "common_math.h"
#include "ds.h"




typedef struct Path {
		
	VEC(Vector2) nodes;
	VEC(float) edgeLengths; // [0] is dist(0, 1)
	
	char isLoop;
	float totalLength;

} Path;



Path* Path_alloc();
void Path_init(Path* p);

void Path_AppendNode(Path* p, Vector2 n);

Vector2 Path_GetPos(Path* p, float dist);


Path* Path_makeRandomLoop(Vector2* center, float radius, int segs, float distort);


#endif // __EACSMB__path_h__
