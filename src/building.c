 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"
#include "game.h"
#include "building.h"





void initBuildings() {
	
	
}







void Building_extrudeAll(Building* b, float height) {
	VEC_EACH(&b->outlines, i, o) {
		Building_extrudeOutline(b, o, height);
	}
}

void Building_extrudeOutline(Building* b, BuildingOutline* o, float height) {
	int i;
	int plen = VEC_LEN(&o->points);
	int base_vertex = VEC_LEN(&b->vertices) - 1;
	
	// add two layers of the outline, offset by the height
	VEC_EACH(&o->points, i, p) {
		VEC_PUSH(&b->vertices, (Vector){p.x, p.y, 0});
	}
	VEC_EACH(&o->points, i, p) {
		VEC_PUSH(&b->vertices, (Vector){p.x, p.y, height});
	}
	
	
	// fill in the triangles
	o->first_index = VEC_LEN(&b->indices) - 1;
	
	for(i = 0; i <= plen; i++) {
		VEC_PUSH(&b->indices, base_vertex + i);
		VEC_PUSH(&b->indices, base_vertex + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen + i);
		
		VEC_PUSH(&b->indices, base_vertex + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen + i);
	}
	
	// stitch up the end to the beginning
	if(o->closed) {
		VEC_PUSH(&b->indices, base_vertex + i);
		VEC_PUSH(&b->indices, base_vertex);
		VEC_PUSH(&b->indices, base_vertex + plen + i);
		
		VEC_PUSH(&b->indices, base_vertex);
		VEC_PUSH(&b->indices, base_vertex + plen);
		VEC_PUSH(&b->indices, base_vertex + plen + i);
	}
	
	o->last_index = VEC_LEN(&b->indices) - 1;
}




void Building_cap(Building* b) {
	
	
	
}






