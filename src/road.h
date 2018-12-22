#ifndef __EACSMB__road_h__
#define __EACSMB__road_h__


#include "common_math.h"

#include "ds.h"



#define LL(x) struct { \
	struct x##_link { \
		x data; \
		struct x##_link* next; \
	}* head; \
} 

// requires GCC for typeof()
#define LL_PUSH(list, x) \
do { \
	typeof((list)->head) LL_tmp__ = malloc(sizeof(*((list)->head))); \
	LL_tmp__->data = x; \
	LL_tmp__->next = (list)->head; \
	(list)->head = LL_tmp__; \
} while(0); \


#define LL_DATA(link) ((link)->data)

#define LL_HEAD(list) LL_DATA((list)->head)

struct RoadEdge;
typedef struct RoadEdge RoadEdge;

typedef struct RoadNode {
	Vector2 pos;
	
	VEC(RoadEdge*) outEdges;
// 	VEC(int) inEdges;
	
	uint32_t eid;
} RoadNode;

typedef struct RoadEdge {
	RoadNode* from, *to;
	float length;
	
	uint32_t eid;
} RoadEdge;


typedef struct RoadNetwork {
	
	VEC(RoadNode*) nodes;
	VEC(RoadEdge*) edges;
	
	
	VEC(RoadEdge*) edgeDirtyList;
	VEC(RoadNode*) nodeDirtyList;
	
} RoadNetwork;





RoadNetwork* RoadNetwork_alloc();
void RoadNetwork_init(RoadNetwork* rn);

RoadNode* RoadNetwork_AddNode(RoadNetwork* rn, Vector2 pos);
void Road_AddEdge(RoadNetwork* rn, RoadNode* from, RoadNode* to);
void Road_AddEdge1Way(RoadNetwork* rn, int from, int to);
Vector2 RoadNetwork_Lerp(RoadNetwork* rn, RoadNode* from, RoadNode* to, float t);
Vector2 RoadNetwork_LerpEdge(RoadNetwork* rn, int e, float t);


RoadNode* RoadNetwork_GetNodeRadius(RoadNetwork* rn, Vector2* pos, float radius);
int RoadNetwork_GetClosestNode(RoadNetwork* rn, Vector2* pos);
int RoadNetwork_GetClosest2Nodes(RoadNetwork* rn, Vector2* pos, int* closest, int* nextClosest);

RoadEdge* RoadNode_GetRandomOutEdge(RoadNetwork* rn, RoadNode* n);



struct World;
void RoadNetwork_FlushDirty(RoadNetwork* rn, struct World* w);


#endif // __EACSMB__road_h__
