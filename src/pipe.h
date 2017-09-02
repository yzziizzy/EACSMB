#ifndef __EACSMB_pipe_h__
#define __EACSMB_pipe_h__


#include "common_math.h"
#include "common_gl.h"

#include "ds.h"
#include "staticMesh.h"


typedef VEC(StaticMeshVertex) SMVList;


typedef struct PipeSegment {
	
	StaticMesh* sm;
	float length;
	
	
} PipeSegment;











void Pipe_init(PipeSegment* ps);






#endif // __EACSMB_pipe_h__
