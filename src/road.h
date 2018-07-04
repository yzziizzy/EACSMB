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




typedef struct RoadNode {
	Vector2 pos;
	
	uint32_t eid;
} RoadNode;

typedef struct RoadEdge {
	int from, to;
	float length;
	
	uint32_t eid;
} RoadEdge;


typedef struct RoadNetwork {
	
	VEC(RoadNode*) nodes;
	VEC(RoadEdge) edges;
	
	
	VEC(int) edgeDirtyList;
	VEC(int) nodeDirtyList;
	
} RoadNetwork;





RoadNetwork* RoadNetwork_alloc();
void RoadNetwork_init(RoadNetwork* rn);

int RoadNetwork_AddNode(RoadNetwork* rn, RoadNode* n); 
void Road_AddEdge(RoadNetwork* rn, int from, int to);
void Road_AddEdge1Way(RoadNetwork* rn, int from, int to);
Vector2 RoadNetwork_Lerp(RoadNetwork* rn, int from, int to, float t);

static inline RoadNode* RoadNetwork_GetNode(RoadNetwork* rn, int index) {
	return VEC_LEN(&rn->nodes) > index && index >= 0 ? VEC_ITEM(&rn->nodes, index) : NULL;
} 


struct World;
void RoadNetwork_FlushDirty(RoadNetwork* rn, struct World* w);


#endif // __EACSMB__road_h__
