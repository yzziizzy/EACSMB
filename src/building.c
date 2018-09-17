 
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
	int i = 0;
	int plen = o->points.length;
	int plen2 = plen * 2;
	int base_vertex = VEC_LEN(&b->vertices);
	
	// add two layers of the outline, offset by the height
	float tdist = 0;
	
	//o->extruded_height = height;
	height = o->extruded_height;
	
	// TODO: extra vertex at the end for texture wrap
	LIST_LOOP(&o->points, p) {
		Vector2* prev = &LIST_PREV_LOOP(&o->points, p)->point;
		Vector2* next = &LIST_NEXT_LOOP(&o->points, p)->point;
		
		// TODO: fix normals
		VEC_PUSH(&b->vertices, ((Vertex_PNT){ 
			p: {p->point.x, p->point.y, 0},
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
		i++;
	}
	
			
// 	VEC_EACH(&b->vertices, i, v) {
// 		printf(")-%d [%.2f,%.2f,%.2f]\n",  i, v.p.x,v.p.y,v.p.z);
// 	}
// 		
	
	// duplicate the vertices vertically
	for(i = 0; i < plen2; i++) {
		Vertex_PNT* p1 = &VEC_ITEM(&b->vertices, base_vertex + i);
	//	Vertex_PNT* p2 = &VEC_ITEM(&b->vertices, i + 1);
		
// 		printf("adding vertex [%.2f,%.2f,%.2f]\n", p1->p.x,p1->p.y,p1->p.z);
		VEC_PUSH(&b->vertices, ((Vertex_PNT){ 
			p: {p1->p.x, p1->p.y, height},
			n: p1->n,
			t: {u: p1->t.u, v: 1},
		}));
		
// 	VEC_EACH(&b->vertices, i2, v) {
// 		printf("%d-%d [%.2f,%.2f,%.2f]\n", i, i2, v.p.x,v.p.y,v.p.z);
// 	}
		
// 		VEC_PUSH(&b->vertices, ((Vertex_PNT){ 
// 			p: {p2->p.x, p2->p.y, height},
// 			n: p2->n,
// 			t: {u: p1->t.v, v: 1},
// 		}));
	}
	
// 	VEC_EACH(&b->vertices, i, v) {
// 		printf("0&%d [%.2f,%.2f,%.2f]\n", i, v.p.x,v.p.y,v.p.z);
// 	}
	
	// fill in the triangles
	o->first_index = VEC_LEN(&b->indices) - 1;
	
	int q = 0;
	for(i = 0; i <= plen; i+=2) {
		VEC_PUSH(&b->indices, base_vertex + i);
		VEC_PUSH(&b->indices, base_vertex + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i);
		
		VEC_PUSH(&b->indices, base_vertex + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i + 1);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i);
		q+=6;
	}
	
// 	VEC_EACH(&b->vertices, i, v) {
// 		printf("&%d [%.2f,%.2f,%.2f]\n", i, v.p.x,v.p.y,v.p.z);
// 	}
	
	// there is a weird bug somewhere in this function
	// sometimes a vertex is corrupted
	// sometimes spurrious extra geometry appears
	// it varies on the amount of input data, but not in a sane way
	
	
	// stitch up the end to the beginning
	if(1 /*o->closed*/) { // only closed loops are allowed atm
		VEC_PUSH(&b->indices, base_vertex + i);
		VEC_PUSH(&b->indices, base_vertex);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i);
		
		VEC_PUSH(&b->indices, base_vertex);
		VEC_PUSH(&b->indices, base_vertex + plen2);
		VEC_PUSH(&b->indices, base_vertex + plen2 + i);
		q+=6;
	}
	
	o->index_count = q;//VEC_LEN(&b->indices) - o->first_index;
}





void Building_capOutline(Building* building, BuildingOutline* o, float height) {
	
	/*
	ear-clipping triangulation algorithm:
	
	take 3 consecutive points
	make sure the triangle they make is clockwise (inside the polygon)
	make sure no other point is inside the triangle
	add the triangle to the list
	remove the middle point from the polygon
	*/
	
	height = o->extruded_height;
	
	// setup
	Vector2_List list;
	LIST_INIT(&list);
	float minx = 9999999, maxx = -9999999, miny = 9999999, maxy = -9999999;
	
	LIST_CONCAT(&o->points, &list);
	
// 	LIST_LOOP(&o->points, p) {
// 		printf("1> [%.2f,%.2f] %d\n", p->point.x,p->point.y, list.length);
// 	}
// 	LIST_LOOP(&list, p) {
// 		printf("2> [%.2f,%.2f] %d\n", p->point.x,p->point.y, list.length);
// 	}
	
	LIST_LOOP(&o->points, p) {
		minx = fmin(minx, p->point.x);
		miny = fmin(miny, p->point.y);
		maxx = fmax(maxx, p->point.x);
		maxy = fmax(maxy, p->point.y);
	}
	
// 	LIST_LOOP(&list, p) {
// 		printf("3> [%.2f,%.2f]\n", p->point.x,p->point.y);
// 	}
// 	
	
	
	float spanx = maxx - minx;
	float spany = maxy - miny;
	
	int ii = 0;
	Vector2_Link* l = list.head;
	
	while(list.length > 2) {
		Vector2* a = &l->point;
		Vector2_Link* bl = LIST_NEXT_LOOP(&list, l);
		Vector2* b = &bl->point;
		Vector2* c = &LIST_NEXT_LOOP(&list, bl)->point;
		float area = triArea2(a, b, c);
		
		// check winding
// 		printf("area: %f\n", area);
		if(area <= 0) { // zero area or counter-clockwise, meaning it's not inside the poly
			// TODO: check for degenerate cases causing infinite loop
// 			printf(" [%.2f,%.2f] [%.2f,%.2f] [%.2f,%.2f]\n", 
// 				a->x,a->y, b->x,b->y, c->x,c->y
// 			);
			printf(" - area skip %f %ul\n", area, l);
			goto CONTINUE;
		}
		
		// ensure no other point is inside this triangle
		// a point inside means the triangle is partially outside the polygon
		Vector2_Link* lp = list.head;
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
	// BUG: stack corruption
 	LIST_FREE(&list);
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
	
	//printf("\n*\n*\nbuilding: vlen: %d, ilen: %d \n", vlen, ilen);
	
	for(i = 0; i < vlen; i++) {
		Vertex_PNT* v = &VEC_ITEM(&b->vertices, i);
		dm->vertices[i] = (DynamicMeshVertex){
			v:  v->p,
			n:  v->n,
			t: {v->t.u * 65536, v->t.v * 65536 },
		};
		
// 		printf("[%.2f, %.2f, %.2f] %d,%d \n", 
// 			dm->vertices[i].v.x,
// 			dm->vertices[i].v.y,
// 			dm->vertices[i].v.z,
// 			dm->vertices[i].t.u,
// 			dm->vertices[i].t.v
// 		);
	}
	
	memcpy(dm->indices.w16, VEC_DATA(&b->indices), ilen * 2);
	
// 	for(i = 0; i < ilen; i++) {
// 		printf("+ %u\t%u\n", (int)VEC_ITEM(&b->indices, i),  (int)dm->indices.w16[i]);
// 	}
	
	dm->defaultScale = 1.0;
	dm->defaultRotX = -3.14159265358979323846264 / 2;
	dm->defaultRotY = 0.0;
	dm->defaultRotZ = 0.0;
	dm->polyMode = GL_TRIANGLES;
	
	return dm;
}










BuildingOutline* BuildingOutline_alloc(float h_offset, float ex_height) {
	BuildingOutline* bo;
	
	pcalloc(bo);
	
	LIST_INIT(&bo->points);
	bo->closed = 1;
	bo->h_offset = h_offset;
	bo->extruded_height = ex_height;
	
	return bo;
}

void BuildingOutline_addPoint(BuildingOutline* bo, Vector2 pos) {
	LIST_APPEND(&bo->points, point, pos);
}

BuildingOutline* BuildingOutline_rect(float h_offset, float ex_height, Vector2 pos, Vector2 size) {
	BuildingOutline* bo;
	
	bo = BuildingOutline_alloc(h_offset, ex_height);
	
	float xh = size.x / 2;
	float yh = size.y / 2;
	
	LIST_APPEND(&bo->points, point, ((Vector2){pos.x - xh, pos.y - yh}));
	LIST_APPEND(&bo->points, point, ((Vector2){pos.x - xh, pos.y + yh}));
	LIST_APPEND(&bo->points, point, ((Vector2){pos.x + xh, pos.y + yh}));
	LIST_APPEND(&bo->points, point, ((Vector2){pos.x + xh, pos.y - yh}));
// 	LIST_APPEND(&bo->points, point, ((Vector2){pos.x + 3 + xh, pos.y - yh - 3}));
// 	LIST_APPEND(&bo->points, point, ((Vector2){pos.x + 6 + xh, pos.y - yh - 9}));
	
	
	// test shape
// 	LIST_APPEND(&bo->points, point, ((Vector2){-10, -10}));
// 	LIST_APPEND(&bo->points, point, ((Vector2){-5, 0}));
// 	LIST_APPEND(&bo->points, point, ((Vector2){-10, 10}));
// 	LIST_APPEND(&bo->points, point, ((Vector2){10, 10}));
// 	LIST_APPEND(&bo->points, point, ((Vector2){-5, -20}));
	
// 	LIST_LOOP(&bo->points, p) {
// 		printf("> [%.2f,%.2f]\n", p->point.x,p->point.y);
// 	}
	
	
	return bo;
}

