
#include "path.h"
#include "utilities.h"






Path* Path_alloc() {
	Path* p;
	
	pcalloc(p);
	Path_init(p);
	
	return p;
}


void Path_init(Path* p) {
	VEC_INIT(&p->nodes);
	VEC_INIT(&p->edgeLengths);
	
}



static void recalcLen(Path* p) {
		
	int i;
	int len = VEC_LEN(&p->nodes); 
	p->totalLength = 0;
	
	if(len < 2)	return;
	
	for(i = 1; i < len + (!!p->isLoop); i++) {
		Vector2 n = VEC_ITEM(&p->nodes, i % len);
		Vector2 on = VEC_ITEM(&p->nodes, i - 1);
		
		float d = vDist2(&on, &n);
		VEC_ITEM(&p->edgeLengths, i - 1) = d; 
		
		p->totalLength += d;	
	}
}

// not the most efficient way of doing things. easy on thinking though
void Path_AppendNode(Path* p, Vector2 n) {
	VEC_PUSH(&p->nodes, n);
	VEC_INC(&p->edgeLengths);
	
	if(p->isLoop && VEC_LEN(&p->nodes) == 2) {
		VEC_INC(&p->edgeLengths);
	}
	
	recalcLen(p);
}


Vector2 Path_GetPos(Path* p, float dist) {
	int i;
	float b;
	float a = 0;
	Vector2 out;
	
	if(VEC_LEN(&p->nodes) == 0) return (Vector2){0 , 0};
	if(VEC_LEN(&p->nodes) == 1) return VEC_ITEM(&p->nodes, 1);
	
	dist = fmod(dist, p->totalLength);
	
	for(i = 0; i < VEC_LEN(&p->edgeLengths); i++) {
		b = VEC_ITEM(&p->edgeLengths, i);
		if(b + a > dist) break;
		a += b;
	}
	
	float t = (dist - a) / b;
	vLerp2(&VEC_ITEM(&p->nodes, i), &VEC_ITEM(&p->nodes, (i + 1) % VEC_LEN(&p->nodes)), t, &out);
	
	return out;
}



Path* Path_makeRandomLoop(Vector2* center, float radius, int segs, float distort) {
	float d = distort * radius;
	float dtheta = F_2PI / (float)segs;
	int i;
	Path* path = Path_alloc();
	path->isLoop = 1;
	
	for(i = 0; i < segs; i++) {
		Path_AppendNode(path, (Vector2){
			center->x + (radius * sin(i * dtheta)) + frand(-d, d),
			center->y + (radius * cos(i * dtheta)) + frand(-d, d)
		});
	}
	
	
	return path;
}



