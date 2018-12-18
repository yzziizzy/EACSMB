#ifndef __EACSMB_pipe_h__
#define __EACSMB_pipe_h__


#include "common_math.h"
#include "common_gl.h"

#include "ds.h"
#include "dynamicMesh.h"

struct World;
typedef struct World World;

// typedef VEC(StaticMeshVertex) SMVList;


typedef struct PipeSegment {
	Vector start, end;
	float length;
} PipeSegment;

typedef struct PipeJoint {
	Vector pos;
	Vector dir;
} PipeJoint;


struct Item;
typedef struct Item Item;

typedef struct PipeLine {
	
	Item* pipe;
	Item* joint;
	
	float length;
	
	float pipeScale;
	float pipeLength;
	float jointScale;
	
	
	VEC(PipeSegment) segments;
	VEC(PipeJoint) joints;
	
	
} PipeLine;


PipeLine* PipeLine_alloc();
void PipeLine_init(PipeLine* pl);


void PipeLine_setMeshes(PipeLine* pl, Item* pipe, Item* joint);




// old
PipeLine* Pipe_create(Vector* points, int numpts);

void Pipe_init(PipeSegment* ps);






#endif // __EACSMB_pipe_h__
