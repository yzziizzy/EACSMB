#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"

#include "utilities.h"

#include "outline.h"




struct outline* outline_alloc(int node_cnt) {
	struct outline* o;
	
	o = calloc(1, sizeof(struct outline));
	outline_init(o, node_cnt);
	
	return o;
}

void outline_init(struct outline* o, int node_cnt) {

	o->nodes = calloc(1, sizeof(struct outline_node) * node_cnt);
	o->len = 0;
	o->alloc = node_cnt;
}

int outline_add_node(struct outline* o, struct outline_node* n) {
	
	if(o->len >= o->alloc) return 1;
	
	memcpy(&o->nodes[len], n, sizeof(struct outline_node));
	o->len++;
	
	return 0;
}












