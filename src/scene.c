#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"


#include "utilities.h"
#include "objloader.h"
#include "shader.h"
#include "staticMesh.h"
#include "emitter.h"
#include "scene.h"
#include "map.h"





void scene_init(Scene* sc) {
	
	initMap(&sc->map);
	
}


/*

static void nodeInsertItem(QuadTree* qt, QuadTreeNode* n, Renderable* r);
static RenderableList* rlPriceIsRight(RenderableList* l, Renderable* r);
static RenderableList* rlInsert(RenderableList* l, Renderable* r);



void initScene(Scene* sc) {
	
	initQuadTree(&sc->qt, 1024, 1024);
	
	
};



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


QuadTreeNode* allocQTNode(QuadTreeNode* parent, char ix, char iy) {
	
	QuadTreeNode* n = calloc(sizeof(QuadTreeNode), 1);
	
	n->parent = parent;
	n->level = parent->level + 1;
	
	// this function calculates the new child's bounding box
	boxQuadrant2(&parent->aabb, ix, iy, &n->aabb);
}


// external interface for adding an item into a tree
void qtInsertItem(QuadTree* qt, Renderable* r) {
	
	nodeInsertItem(qt, qt->root, r);
	qt->totalCount++;
}



static void nodeInsertItem(QuadTree* qt, QuadTreeNode* n, Renderable* r) {
	RenderableList* i, *j;
	
	if(n->itemCount == -1) { // signifies internal node, recurse deeper
		nodeInsertItem(qt, n->kids[0][0], r);
		nodeInsertItem(qt, n->kids[0][1], r);
		nodeInsertItem(qt, n->kids[1][0], r);
		nodeInsertItem(qt, n->kids[1][1], r);
		return;
	}
	
	// actually check if the item falls in this node
	// the 3D->2D cast is a bit evil but works fine. think of it like casting a short into an int. 
	if(!boxOverlaps2(&n->aabb, (AABB2*)&r->aabb)) return;
	
	// just add the item if this node is still small
	if(n->itemCount < qt->nodeMaxCount || n->level == qt->maxLevels) {
		n->items = rlInsert(n->items, r);
		n->itemCount++;
		return;
	}
	
	// split the node
	n->itemCount == -1;
	n->kids[0][0] = allocQTNode(n, 0, 0);
	n->kids[0][1] = allocQTNode(n, 0, 1);
	n->kids[1][0] = allocQTNode(n, 1, 0);
	n->kids[1][1] = allocQTNode(n, 1, 1);
	
	// the current item
	nodeInsertItem(qt, n->kids[0][0], r);
	nodeInsertItem(qt, n->kids[0][1], r);
	nodeInsertItem(qt, n->kids[1][0], r);
	nodeInsertItem(qt, n->kids[1][1], r);
	
	// existing items, free the list too
	i = n->items;
	while(i) {
		nodeInsertItem(qt, n->kids[0][0], i->r);
		nodeInsertItem(qt, n->kids[0][1], i->r);
		nodeInsertItem(qt, n->kids[1][0], i->r);
		nodeInsertItem(qt, n->kids[1][1], i->r);
		
		j = i;
		i = i->next;
		free(j);
	}
	
	n->items = NULL;
}


// highest without going over, 0 if the first is higher or l is null
static RenderableList* rlPriceIsRight(RenderableList* l, Renderable* r) {
	
	RenderableList* prev = 0;
	
	while(l && l->r >= r) {
		prev = l;
		l = l->next;
	}
	
	return prev;
}

// inserted sorted by r's pointer value, returns the new list root
static RenderableList* rlInsert(RenderableList* l, Renderable* r) {
	
	RenderableList* new, *prev;
	
	new = malloc(sizeof(RenderableList));
	new->r = r;
	
	prev = rlPriceIsRight(l, r);
	
	if(prev) {
		new->next = prev->next;
		prev->next = new;
		return l;
	}
	
	// insert as the new head of the list (or only item)
	new->next = l;
	return new;
}



*/











