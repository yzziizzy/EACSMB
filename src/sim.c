
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"

#include "utilities.h"
#include "sim.h"




TransGraph* allocTransGraph() {
	return calloc(1, sizeof(TransGraph));
}

void initTransGraph(TransGraph* tg) {
	tg->maxNode = 1<<12;
	tg->maxEdge = 1<<14;
}

// add edge
// rm edge
// distance

int addTransNode(TransGraph* tg, uint16_t type, Vector2* pos, uint16_t* out_id) {
	int id = tg->nextNode;
	
	if(id >= tg->maxNode) return 1; 
	
	tg->nodes[id].type = type;
	vCopy2(&pos, &tg->nodes[id].pos);
	
	tg->nextNode++;
	*out_id = id;
	
	return 0;
}


int addTransEdge(TransGraph* tg, uint16_t from, uint16_t to) {
	int id = tg->nextEdge;
	
	if(id >= tg->maxEdge) return 1; 
	if(from >= tg->nextNode) return 1; 
	if(to >= tg->nextNode) return 1; 
	
	tg->edges[id].from = from;
	tg->edges[id].to = to;
	
	tg->nextEdge++;
	
	return 0;
}










