#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c_json/json.h"
#include "json_gl.h"

#include "utilities.h"
#include "shader.h"
#include "staticMesh.h"
#include "emitter.h"
#include "scene.h"
#include "map.h"
#include "fbo.h"



// // for easy pooling later
// SceneItemInfo* SceneItemInfo_alloc() {
// 	return calloc(1, sizeof(SceneItemInfo)); 
// }

/*

static void unpack_fbo(json_value_t* p, char* key, FBOTexConfig* cfg) {
	char* a, *b, *c;
	json_value_t* o, *v1, *v2, *v3;
	
	json_obj_get_key(p, key, &o);
	
	json_obj_get_key(o, "internalType", &v1); a = v1->v.str;
	json_as_GLenum(v1, &cfg->internalType);
	
	json_obj_get_key(o, "format", &v2); b = v2->v.str;
	json_as_GLenum(v2, &cfg->format);
	
	json_obj_get_key(o, "size", &v3); c = v3->v.str;
	json_as_GLenum(v3, &cfg->size);
	
	printf("fbo cfg from json: %s: %x, %s: %x, %s: %x\n", a, cfg->internalType, b, cfg->format, c, cfg->size);
}

static float random_from_array(json_value_t* arr) {
	float f, range, min, max;
	
	if(arr->type != JSON_TYPE_ARRAY) return 0.0;
	
	json_as_float(arr->v.arr->head->value, &min);
	json_as_float(arr->v.arr->tail->value, &max);
	if(min > max) {
		f = min;
		min = max;
		max = min;
	}
	range = max - min;
	
	if(range == 0.0) return min;
	f = (float)rand()/(float)(RAND_MAX/range);
	
	return f;
}

static void random_vector_from_array(json_value_t* arr, Vector* vec) {
	
	if(arr->type != JSON_TYPE_ARRAY) return 0.0;
	
	json_as_float(arr->v.arr->head->value, &vec->x);
	json_as_float(arr->v.arr->head->next->value, &vec->y);
	json_as_float(arr->v.arr->tail->value, &vec->z);
}



struct EmitterSprite_gen_info {
	Vector start_pos;
	float offset;
	
	Vector start_vel;
	float spawn_delay;
	
	Vector start_acc;
	float lifetime;
	
	float size, spin, growth_rate, randomness;
	
	float fade_in, fade_out, unallocated_1, unallocated_2;
};

*/

void Scene_init(Scene* sc) {
	
	// TODO: rename to new conventions
	//initMap(&sc->map);
	
	VEC_INIT(&sc->allDrawables);
	
	
	// solids
	
	// decals
	
	// alpha
	
	// TODO: shading
	
	
	
	
	
}


/*

void initScene(Scene* sc) {
	
	initQuadTree(&sc->qt, 1024, 1024);
	
	
};
*/
/*
static void qtnode_insert(QuadTree* qt, QuadTreeNode* n, SceneItemInfo* info);


void initQuadTree(QuadTree* qt, float szX, float szY) {
	
	// initialize the root node; the normal node alloc function can't handle not having a parent.
	qt->root = calloc(sizeof(QuadTreeNode), 1);
	qt->root->aabb.min.x = -(szX / 2);
	qt->root->aabb.max.x = (szX / 2);
	qt->root->aabb.min.y = -(szY / 2);
	qt->root->aabb.max.y = (szY / 2);
	
	qt->maxLevels = 8;
	qt->nodeMinCount = 8;
	qt->nodeMinCount = 16;
	
	qt->totalCount = 0;
}


QuadTreeNode* QTNode_alloc(QuadTreeNode* parent, char ix, char iy) {
	
	QuadTreeNode* n = calloc(sizeof(QuadTreeNode), 1);
	
	QTNode_init(n, parent, ix, iy);
	
	return n;
}

void QTNode_init(QuadTreeNode* n, QuadTreeNode* parent, char ix, char iy) {
	
	n->parent = parent;
	n->level = parent->level + 1;
	
	// this function calculates the new child's bounding box
	boxQuadrant2(&parent->aabb, ix, iy, &n->aabb);
	
	VEC_INIT(&n->items);
}


void QuadTree_insert(QuadTree* qt, SceneItemInfo* info) {
	
	qtnode_insert(qt, qt->root, info);
	
	qt->totalCount++;
}




static void qtnode_insert(QuadTree* qt, QuadTreeNode* n, SceneItemInfo* info) {
	
	if(!boxOverlaps2(&n->aabb, &info->aabb)) return;
	
	
	// just add the item if this node is still small
// 	if(n->itemCount < qt->nodeMaxCount || n->level == qt->maxLevels) {
// 		n->items = rlInsert(n->items, info);
// 		n->itemCount++;
// 		return;
// 	}
}



static void nodeInsertItem(QuadTree* qt, QuadTreeNode* n, SceneItemInfo* sii) {
// 	RenderableList* i, *j;
// 	
// 	if(n->itemCount == -1) { // signifies internal node, recurse deeper
// 		nodeInsertItem(qt, n->kids[0][0], sii);
// 		nodeInsertItem(qt, n->kids[0][1], sii);
// 		nodeInsertItem(qt, n->kids[1][0], sii);
// 		nodeInsertItem(qt, n->kids[1][1], sii);
// 		return;
// 	}
// 	
// 	// actually check if the item falls in this node
// 	// the 3D->2D cast is a bit evil but works fine. think of it like casting a short into an int. 
// 	if(!boxOverlaps2(&n->aabb, (AABB2*)&r->aabb)) return;
// 	
// 	// just add the item if this node is still small
// // 	if(n->itemCount < qt->nodeMaxCount || n->level == qt->maxLevels) {
// // 		n->items = rlInsert(n->items, sii);
// // 		n->itemCount++;
// // 		return;
// // 	}
// 	
// 	// split the node
// 	n->itemCount == -1;
// 	n->kids[0][0] = allocQTNode(n, 0, 0);
// 	n->kids[0][1] = allocQTNode(n, 0, 1);
// 	n->kids[1][0] = allocQTNode(n, 1, 0);
// 	n->kids[1][1] = allocQTNode(n, 1, 1);
// 	
// 	// the current item
// // 	nodeInsertItem(qt, n->kids[0][0], r);
// // 	nodeInsertItem(qt, n->kids[0][1], r);
// // 	nodeInsertItem(qt, n->kids[1][0], r);
// // 	nodeInsertItem(qt, n->kids[1][1], r);
// 	
// 	// existing items, free the list too
// 	i = n->items;
// 	while(i) {
// // 		nodeInsertItem(qt, n->kids[0][0], i->r);
// // 		nodeInsertItem(qt, n->kids[0][1], i->r);
// // 		nodeInsertItem(qt, n->kids[1][0], i->r);
// // 		nodeInsertItem(qt, n->kids[1][1], i->r);
// 		
// 		j = i;
// 		i = i->next;
// 		free(j);
// 	}
// 	
// 	n->items = NULL;
}


*/







