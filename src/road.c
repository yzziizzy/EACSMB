



#include "road.h"
#include "world.h"
#include "utilities.h"





//
static void foo() {
	
	LL(int) l;
	
	LL_PUSH(&l, 3); 
	
	printf("foo %d\n", LL_HEAD(&l));
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
	
	VEC_INIT(&rn->nodeDirtyList);
	VEC_INIT(&rn->edgeDirtyList);
}



int RoadNetwork_AddNode(RoadNetwork* rn, RoadNode* n) {
	int index = VEC_LEN(&rn->nodes);
	VEC_PUSH(&rn->nodes, n);
	
	return index;
}




// bidirection
void Road_AddEdge(RoadNetwork* rn, int from, int to) {
	RoadNode* fn = RoadNetwork_GetNode(rn, from);
	RoadNode* tn = RoadNetwork_GetNode(rn, to);
	
	float l = vDist2(&fn->pos, &tn->pos);
	VEC_PUSH(&rn->edges, ((RoadEdge){.from = from, .to = to, .length = l}));
	VEC_PUSH(&rn->edges, ((RoadEdge){.from = to, .to = from, .length = l}));
	
	int i = VEC_LEN(&rn->edges) - 1;
	VEC_PUSH(&rn->edgeDirtyList, i);
	VEC_PUSH(&rn->edgeDirtyList, i - 1);
	VEC_PUSH(&rn->nodeDirtyList, from);
	VEC_PUSH(&rn->nodeDirtyList, to);
}

// unidirectional
void Road_AddEdge1Way(RoadNetwork* rn, int from, int to) {
	RoadNode* fn = RoadNetwork_GetNode(rn, from);
	RoadNode* tn = RoadNetwork_GetNode(rn, to);
	
	printf("from %d to %d \n", from , to);
	
	float l = vDist2(&fn->pos, &tn->pos);
	VEC_PUSH(&rn->edges, ((RoadEdge){.from = from, .to = to, .length = l}));
	
	int i = VEC_LEN(&rn->edges) - 1;
	VEC_PUSH(&rn->edgeDirtyList, i);
	VEC_PUSH(&rn->nodeDirtyList, from);
	VEC_PUSH(&rn->nodeDirtyList, to);
}




Vector2 RoadNetwork_Lerp(RoadNetwork* rn, int from, int to, float t) {
	RoadNode* fn = RoadNetwork_GetNode(rn, from);
	RoadNode* tn = RoadNetwork_GetNode(rn, to);
	Vector2 out;
	
	vLerp2(&fn->pos, &tn->pos, t, &out);
	
	return out;
}


// -1 for not found
int RoadNetwork_GetNodeRadius(RoadNetwork* rn, Vector2* pos, float radius) {
	int closest = -1;
	float cdist = 99999999.0; 
	
	if(VEC_LEN(&rn->nodes) < 1) return -1;
		
	VEC_EACH(&rn->nodes, i, n) {
		float d = vDist2(pos, &n->pos);
		//printf("dist: %d - %f - %f,%f -> %f,%f\n", i, d, pos->x, pos->y, n->pos.x, n->pos.y);
		if(d <= radius && d < cdist) {
			cdist = d;
			closest = i;
		}
	} 
	
	return closest;
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
	RoadNode* fn = RoadNetwork_GetNode(rn, e->from);
	RoadNode* tn = RoadNetwork_GetNode(rn, e->to);
	
	printf("es from %d to %d\n", e->from, e->to);
	
	World_spawnAt_CustomDecal(w, 0, 2, &fn->pos, &tn->pos);

}



// this function recalculates all metadata for the dirty lists
void RoadNetwork_FlushDirty(RoadNetwork* rn, struct World* w) {
	
	VEC_LOOP(&rn->edgeDirtyList, i) {
		int ei = VEC_ITEM(&rn->edgeDirtyList, i);
		RoadEdge* e = &VEC_ITEM(&rn->edges, ei);
		
		RoadEdge_spawnVisuals(rn, e, w);
	}
	
	VEC_TRUNC(&rn->edgeDirtyList);
	
	
	
	
	VEC_TRUNC(&rn->nodeDirtyList);
}



