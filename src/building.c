 
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
	int plen2 = plen * 2;
	int base_vertex = VEC_LEN(&b->vertices);
	
	// add two layers of the outline, offset by the height
	float tdist = 0;
	
	o->extruded_height = height;
	
	// TODO: extra vertex at the end for texture wrap
	VEC_EACH(&o->points, i, p) {
		Vector2* prev = &VEC_ITEM(&o->points, (i + plen - 1) % plen);
		Vector2* next = &VEC_ITEM(&o->points, (i + plen + 1) % plen);
		
		// TODO: fix normals
		VEC_PUSH(&b->vertices, ((Vertex_PNT){ 
			p: {p.x, p.y, 0},
			n: {0,0,0},
			t: {u: tdist, v: 0},
		}));
		
		// two vertices for hard creases
		VEC_PUSH(&b->vertices, ((Vertex_PNT){ 
			p: {next->x, next->y, 0},
			n: {0,0,0},
			t: {u: tdist, v: 0},
		}));
		
		tdist += vDist2(&p, next);
	}
	
	// duplicate the vertices vertically
	for(i = 0; i < plen2; i += 2) {
		Vertex_PNT* p1 = &VEC_ITEM(&b->vertices, i);
		Vertex_PNT* p2 = &VEC_ITEM(&b->vertices, i + 1);
		
		VEC_PUSH(&b->vertices, ((Vertex_PNT){ 
			p: {p1->p.x, p1->p.y, height},
			n: p1->n,
			t: {u: p1->t.u, v: 1},
		}));
		VEC_PUSH(&b->vertices, ((Vertex_PNT){ 
			p: {p2->p.x, p2->p.y, height},
			n: p2->n,
			t: {u: p1->t.v, v: 1},
		}));
	}
	
	
	
	// fill in the triangles
	o->first_index = VEC_LEN(&b->indices) - 1;
	
	printf("> plen: %d \n", plen);
	for(i = 0; i <= plen+1; i += 2) {
		VEC_PUSH(&b->indices, base_vertex + i);
		VEC_PUSH(&b->indices, base_vertex + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i);
		
		VEC_PUSH(&b->indices, base_vertex + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i);
	}
	
	// stitch up the end to the beginning
	if(1 /*o->closed*/) { // only closed loops are allowed atm
		VEC_PUSH(&b->indices, base_vertex + i);
		VEC_PUSH(&b->indices, base_vertex);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i);
		
		VEC_PUSH(&b->indices, base_vertex);
		VEC_PUSH(&b->indices, base_vertex + plen2);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i);
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
	if(__new_link->prev) __new_link->prev->next = __new_link; \
	(list)->tail = __new_link; \
	if((list)->head == NULL) (list)->head = __new_link; \
	__new_link->prop = (x); \
	(list)->length++; \
} while(0);


#define LIST_PREPEND(list, prop, x) \
do { \
	typeof((list)->head) __new_link = calloc(1, sizeof(*__new_link)); \
	__new_link->next = (list)->head; \
	if(__new_link->next) __new_link->next->prev = __new_link; \
	(list)->head = __new_link; \
	if((list)->tail == NULL) (list)->tail = __new_link; \
	__new_link->prop = x; \
	(list)->length++; \
} while(0);

#define LIST_REMOVE(list, link) \
do { \
	typeof((list)->head) __link = (link); \
	if((__link) == (list)->head) (list)->head = (__link)->next; \
	if((__link) == (list)->tail) (list)->tail = (__link)->prev; \
	if((__link)->prev) (__link)->prev->next = (__link)->next; \
	if((__link)->next) (__link)->next->prev = (__link)->prev; \
	free(__link); \
	(list)->length = (list)->length == 0 ? 0 : (list)->length - 1; \
} while(0);

#define LIST_NEXT_LOOP(list, link) \
((link)->next ? (link)->next : (list)->head)




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
	float minx = 9999999, maxx = -9999999, miny = 9999999, maxy = -9999999;
	
	VEC_EACH(&o->points, i, p) {
		LIST_APPEND(&list, point, p);
		minx = fmin(minx, p.x);
		miny = fmin(miny, p.y);
		maxx = fmax(maxx, p.x);
		maxy = fmax(maxy, p.y);
	}
	
	
	float spanx = maxx - minx;
	float spany = maxy - miny;
	
	int ii = 0;
	Link* l = list.head;
	
	while(list.length > 2) {
		Vector2* a = &l->point;
		Link* bl = LIST_NEXT_LOOP(&list, l);
		Vector2* b = &bl->point;
		Vector2* c = &LIST_NEXT_LOOP(&list, bl)->point;
		float area = triArea2(a, b, c);
		
// 		printf("ii: %d\n",ii++);
// 		fflush(stdout);
// 		LIST_REMOVE(&list, bl);
// 		continue;
		
		// check winding
		printf("area: %f\n", area);
		if(area <= 0) { // zero area or counter-clockwise, meaning it's not inside the poly
			// TODO: check for degenerate cases causing infinite loop
			printf(" - area skip %f %ul\n", area, l);
			goto CONTINUE;
		}
		
		// ensure no other point is inside this triangle
		// a point inside means the triangle is partially outside the polygon
		Link* lp = list.head;
		do {
			if(lp != bl && lp != bl->next && lp != l) {
				if(triPointInside2(&lp->point, a, b, c)) {
					printf("point inside");
					goto CONTINUE;
				}
				else printf("outside\n");
			}
			else printf("in tri\n");
			
			lp = lp->next;
		} while(lp->next);
		
		
		// the triangle is an ear. clip and save it like a free-item harbor freight coupon
		
		// save
		int base_vertex = VEC_LEN(&building->vertices);
		
		// new vertices for normals
		// TODO: uv's?
		VEC_PUSH(&building->vertices, ((Vertex_PNT){
			p: {a->x, a->y, height}, 
			n: {0,0,1}, 
			t: {(a->x - minx) / spanx, (a->y - miny) / spany}
		}));
		VEC_PUSH(&building->vertices, ((Vertex_PNT){
			p: {b->x, b->y, height}, 
			n: {0,0,1}, 
			t: {(b->x - minx) / spanx, (b->y - miny) / spany}
		}));
		VEC_PUSH(&building->vertices, ((Vertex_PNT){
			p: {c->x, c->y, height}, 
			n: {0,0,1}, 
			t: {(c->x - minx) / spanx, (c->y - miny) / spany}
		}));
		
		VEC_PUSH(&building->indices, base_vertex + 0);
		VEC_PUSH(&building->indices, base_vertex + 1);
		VEC_PUSH(&building->indices, base_vertex + 2);
		
	
		// clip out the middle vertex
		LIST_REMOVE(&list, bl);
		
		
	CONTINUE:
		// this purposefully rotates around the polygon on success to avoid creating fans 
		// fans have a tendency to create slender or degenerate triangles
		l = l->next ? l->next : list.head;
		printf(" - %d %d continue %ul\n", ii++, list.length, l);
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






DynamicMesh* Building_CreateDynamicMesh(Building* b) {
	int i;
	DynamicMesh* dm;
	pcalloc(dm);
	
	dm->texIndex = 0;
	
	int vlen = VEC_LEN(&b->vertices);
	int ilen = VEC_LEN(&b->indices);
	
	dm->vertices = malloc(vlen * sizeof(*dm->vertices));
	dm->vertexCnt = vlen;
	dm->indices.w16 = malloc(ilen * sizeof(unsigned short));
	dm->indexCnt = ilen;
	dm->indexWidth = 2;
	
	printf("\n*\n*\nbuilding: vlen: %d, ilen: %d \n", vlen, ilen);
	
	for(i = 0; i < vlen; i++) {
		Vertex_PNT* v = &VEC_ITEM(&b->vertices, i);
		dm->vertices[i] = (DynamicMeshVertex){
			v:  v->p,
			n:  v->n,
			t: {v->t.u * 65536, v->t.v * 65536 },
		};
		
		printf("[%.2f, %.2f, %.2f] %d,%d \n", 
			dm->vertices[i].v.x,
			dm->vertices[i].v.y,
			dm->vertices[i].v.z,
			dm->vertices[i].t.u,
			dm->vertices[i].t.v
		);
	}
	
	memcpy(dm->indices.w16, VEC_DATA(&b->indices), ilen * 2);
	
	for(i = 0; i < ilen; i++) {
		printf("+ %u\t%u\n", (int)VEC_ITEM(&b->indices, i),  (int)dm->indices.w16[i]);
	}
	
	dm->defaultScale = 1.0;
	dm->defaultRotX = -3.14159265358979323846264 / 2;
	dm->defaultRotY = 0.0;
	dm->defaultRotZ = 0.0;
	dm->polyMode = GL_TRIANGLES;
	
	return dm;
}



