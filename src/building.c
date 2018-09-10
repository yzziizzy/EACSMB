 
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
		VEC_PUSH(&b->vertices, ((Vector){p.x, p.y, 0}));
	}
	VEC_EACH(&o->points, i, p) {
		VEC_PUSH(&b->vertices, ((Vector){p.x, p.y, height}));
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
	
	o->index_count = VEC_LEN(&b->indices) - o->first_index;
}





typedef struct Link {
	struct Link* next;
	struct Link* prev;
	
	Vector2 point;
} Link;

typedef struct List {
	Link* head;
	Link* tail;
	int length;
} List;



#define LIST_APPEND(list, prop, x) \
do { \
	typeof((list)->head) __new_link = calloc(1, sizeof(*__new_link)); \
	__new_link->prev = (list)->tail; \
	(list)->tail = __new_link; \
	if((list)->head == NULL) (list)->head = __new_link; \
	__new_link->prop = x; \
	(list)->length++; \
} while(0);


#define LIST_PREPEND(list, prop, x) \
do { \
	typeof((list)->head) __new_link = calloc(1, sizeof(*__new_link)); \
	__new_link->next = (list)->head; \
	(list)->head = __new_link; \
	if((list)->tail == NULL) (list)->tail = __new_link; \
	__new_link->prop = x; \
	(list)->length++; \
} while(0);

#define LIST_REMOVE(list, link) \
do { \
	if((link)->prev) (link)->prev->next = (link)->next; \
	if((link)->next) (link)->next->prev = (link)->prev; \
	if((link) == (list)->head) (list)->head == (link)->next; \
	if((link) == (list)->tail) (list)->tail == (link)->prev; \
	free(link); \
	(list)->length = (list)->length == 0 ? 0 : (list)->length - 1; \
} while(0);






void Building_capOutline(Building* building, BuildingOutline* o, float height) {
	
	/*
	ear-clipping triangulation algorithm:
	
	take 3 consecutive points
	make sure the triangle they make is clockwise (inside the polygon)
	make sure no other point is inside the triangle
	add the triangle to the list
	remove the middle point from the polygon
	*/
	
	
	// setup
	List list;
	list.head = NULL;
	list.tail = NULL;
	
	VEC_EACH(&o->points, i, p) {
		LIST_APPEND(&list, point, p);
	}
	
	
	Link* l = list.head;
	while(list.length > 2) {
		Vector2* a = &l->point;
		Vector2* b = &l->next->point;
		Vector2* c = &l->next->next->point;
		float area = triArea2(a, b, c);
		
		// check winding
		if(area <= 0) { // zero area or counter-clockwise, meaning it's not inside the poly
			// TODO: check for degenerate cases causing infinite loop
			continue;
		}
		
		// ensure no other point is inside this triangle
		// a point inside means the triangle is partially outside the polygon
		Link* lp = list.head;
		do {
			if(triPointInside2(&lp->point, a, b, c)) goto CONTINUE;
		
			lp = lp->next;
		} while(lp->next);
		
		
		// the triangle is an ear. clip and save it like a free-item harbor freight coupon
		
		// save
		int base_vertex = VEC_LEN(&building->vertices) - 1;
		VEC_PUSH(&building->vertices, ((Vector){a->x, a->y, height}));
		VEC_PUSH(&building->vertices, ((Vector){b->x, b->y, height}));
		VEC_PUSH(&building->vertices, ((Vector){c->x, c->y, height}));
		VEC_PUSH(&building->indices, base_vertex + 0);
		VEC_PUSH(&building->indices, base_vertex + 1);
		VEC_PUSH(&building->indices, base_vertex + 2);
		
		// clip out the middle vertex
		LIST_REMOVE(&list, l->next);
		
		
	CONTINUE:
		// this purposefully rotates around the polygon on success to avoid creating fans 
		// fans have a tendency to create slender or degenerate triangles
		l = l->next ? l->next : list.head;
	}
	
	if(list.length != 2) {
		fprintf(stderr, "!!! list->length != 2 in building cap generator.\n");
	}
	
	// free the last two links
	LIST_REMOVE(&list, list.head);
	LIST_REMOVE(&list, list.head);
}


void Building_capAll(Building* b, float height) {
	VEC_EACH(&b->outlines, i, o) {
		Building_capOutline(b, o, height);
	}
}




