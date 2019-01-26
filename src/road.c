



#include "road.h"
#include "world.h"
#include "utilities.h"





static int isOutEdge(const RoadEdge* const e, const RoadNode* const from) {
	return (e->flags & ROADFLAG_2WAY || from == e->from);
} 

static int isInEdge(const RoadEdge* const e, const RoadNode* const to) {
	return (e->flags & ROADFLAG_2WAY || to == e->to);
} 





RoadNetwork* RoadNetwork_alloc() {
	RoadNetwork* rn;
	
	pcalloc(rn);
	RoadNetwork_init(rn);
	
	return rn;
}

void RoadNetwork_init(RoadNetwork* rn) {
	VEC_INIT(&rn->nodes);
	VEC_INIT(&rn->edges);
	
// 	VEC_INIT(&rn->nodeDirtyList);
// 	VEC_INIT(&rn->edgeDirtyList);
}



RoadNode* RoadNetwork_AddNode(RoadNetwork* rn, Vector2 pos) {
	RoadNode* n;
	
	pcalloc(n);
	n->pos = pos;
	VEC_INIT(&n->outEdges);
// 	VEC_INIT(&n->inEdges);
	
	VEC_PUSH(&rn->nodes, n);
	
	return n;
}



RoadEdge* RoadNode_GetRandomOutEdge(RoadNetwork* rn, RoadNode* n) {
	int len = VEC_LEN(&n->outEdges);
	if(len == 0) return NULL;
	int i = rand() % len;
	
	return VEC_ITEM(&n->outEdges, i);
}


RoadEdge* RoadEdge_alloc(RoadNode* from, RoadNode* to) {
	RoadEdge* e = calloc(1, sizeof(*e));
	
	e->from = from;
	e->to = to; 
	e->length = vDist2(&from->pos, &to->pos);
	
	return e;
}

// bidirection
void Road_AddEdge(RoadNetwork* rn, RoadNode* from, RoadNode* to) {
	
	RoadEdge* e = RoadEdge_alloc(from, to);
	e->flags |= ROADFLAG_2WAY;
	
	float l = vDist2(&from->pos, &to->pos);
	VEC_PUSH(&rn->edges, e);
	VEC_PUSH(&from->outEdges, e);
	VEC_PUSH(&to->outEdges, e);
	
	VEC_PUSH(&rn->edgeDirtyList, e);
}

// unidirectional
void Road_AddEdge1Way(RoadNetwork* rn, int from, int to) {
// 	RoadNode* fn = RoadNetwork_getNode(rn, from);
// 	RoadNode* tn = RoadNetwork_getNode(rn, to);
// 	
// 	printf("from %d to %d \n", from , to);
// 	
// 	float l = vDist2(&fn->pos, &tn->pos);
// 	VEC_PUSH(&rn->edges, ((RoadEdge){.from = from, .to = to, .length = l}));
// 	
// 	int i = VEC_LEN(&rn->edges) - 1;
// 	VEC_PUSH(&rn->edgeDirtyList, i);
// 	VEC_PUSH(&rn->nodeDirtyList, from);
// 	VEC_PUSH(&rn->nodeDirtyList, to);
}



Vector2 RoadNetwork_LerpEdge(RoadNetwork* rn, int e, float t) {
// 	RoadEdge* edge = RoadNetwork_getEdge(rn, e);
// 	RoadNode* fn = RoadNetwork_getNode(rn, edge->from);
// 	RoadNode* tn = RoadNetwork_getNode(rn, edge->to);
// 	Vector2 out;
	
// 	vLerp2(&fn->pos, &tn->pos, t, &out);
	
// 	return out;
}

Vector2 RoadNetwork_Lerp(RoadNetwork* rn, RoadNode* from, RoadNode* to, float t, char backwards) {
	Vector2 out;
	
	vLerp2(backwards ? &to->pos : &from->pos, backwards ? &from->pos : &to->pos, t, &out);
	
	return out;
}


// -1 for not found
RoadNode* RoadNetwork_GetNodeRadius(RoadNetwork* rn, Vector2* pos, float radius) {
	int closest = -1;
	float cdist = 99999999.0; 
	
	if(VEC_LEN(&rn->nodes) < 1) return NULL;
		
	VEC_EACH(&rn->nodes, i, n) {
		float d = vDist2(pos, &n->pos);
		//printf("dist: %d - %f - %f,%f -> %f,%f\n", i, d, pos->x, pos->y, n->pos.x, n->pos.y);
		if(d <= radius && d < cdist) {
			cdist = d;
			closest = i;
		}
	} 
	
	
	return closest < 0 ? NULL : VEC_ITEM(&rn->nodes, closest);
}

// -1 for not found
int RoadNetwork_GetClosestNode(RoadNetwork* rn, Vector2* pos) {
	int closest = -1;
	float cdist = 99999999.0; 
	
	if(VEC_LEN(&rn->nodes) < 1) return -1;
		
	VEC_EACH(&rn->nodes, i, n) {
		float d = vDist2(pos, &n->pos);
		//printf("dist: %d - %f - %f,%f -> %f,%f\n", i, d, pos->x, pos->y, n->pos.x, n->pos.y);
		if(d < cdist) {
			cdist = d;
			closest = i;
		}
	} 
	
	return closest;
}

// -1 for not found
int RoadNetwork_GetClosest2Nodes(RoadNetwork* rn, Vector2* pos, int* closest, int* nextClosest) {
	int close = -1;
	int next = -1;
	float cdist = 99999999.0; 
	float ndist = 99999999.0; 
	
	if(VEC_LEN(&rn->nodes) < 2) return -1;
		
	VEC_EACH(&rn->nodes, i, n) {
		float d = vDist2(pos, &n->pos);
		//printf("dist: %d - %f - %f,%f -> %f,%f\n", i, d, pos->x, pos->y, n->pos.x, n->pos.y);
		if(d < cdist) {
			ndist = cdist;
			next = close;
			cdist = d;
			closest = i;
		}
		else if(d < ndist) {
			ndist = d;
			next = i;
		}
	} 
	
	if(closest) *closest = close;
	if(next) *nextClosest = next;
	
	return closest;
}






static void RoadEdge_spawnVisuals(RoadNetwork* rn, RoadEdge* e, World* w) {
// printf("es from %d to %d\n", e->from, e->to);
	
	World_spawnAt_CustomDecal(w, 0, 2, &e->from->pos, &e->to->pos);

}



// this function recalculates all metadata for the dirty lists
void RoadNetwork_FlushDirty(RoadNetwork* rn, struct World* w) {
	
	VEC_EACH(&rn->edgeDirtyList, i, e) {
		RoadEdge_spawnVisuals(rn, e, w);
	}
	
	VEC_TRUNC(&rn->edgeDirtyList);
	
	
	
	
	VEC_TRUNC(&rn->nodeDirtyList);
}





RoadEdge* RoadNetwork_FindClosestEdge(RoadNetwork* rn, Vector2 pos) {
	
}

RoadNode* RoadNetwork_FindClosestNode(RoadNetwork* rn, Vector2 pos) {
	
}

void RoadNetwork_FindPath(RoadNetwork* rn, RoadNode* start, RoadNode* finish) {
	
	
	
	
}




