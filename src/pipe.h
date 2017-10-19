#ifndef __EACSMB_pipe_h__
#define __EACSMB_pipe_h__


#include "common_math.h"
#include "common_gl.h"

#include "ds.h"
#include "staticMesh.h"


typedef VEC(StaticMeshVertex) SMVList;


typedef struct PipeSegment {
	Vector start, end;
	float length;
} PipeSegment;

typedef struct PipeJoint {
	Vector pos;
	Vector dir;
} PipeJoint;


typedef struct PipeLine {
	
	StaticMesh* pipe;
	StaticMesh* joint;
	
	float length;
	
	float pipeScale;
	float pipeLength;
	float jointScale;
	
	
	VEC(PipeSegment) segments;
	VEC(PipeJoint) joints;
	
	
} PipeLine;






PipeLine* Pipe_create(Vector* points, int numpts);

void Pipe_init(PipeSegment* ps);






#endif // __EACSMB_pipe_h__
