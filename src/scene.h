#ifndef __EACSMB_SCENE_H__
#define __EACSMB_SCENE_H__

#include "map.h"
#include "staticMesh.h"
#include "emitter.h"

/*
typedef struct Renderable {
	AABB aabb;

	int type;
	void* data;
	
} Renderable;

typedef struct RenderableList {
	Renderable* r;
	struct RenderableList* next;
} RenderableList;


typedef struct QuadTreeNode {
	AABB2 aabb;
	int itemCount;
	int level;
	RenderableList* items;
	
	struct QuadTreeNode* parent;
	struct QuadTreeNode* kids[2][2];
} QuadTreeNode;


typedef struct QuadTree {
	int maxLevels;
	int nodeMaxCount; // trigger to subdivide
	int nodeMinCount; // trigger to recombine
	
	int totalCount;
	
	
	QuadTreeNode* root;
} QuadTree;
*/



typedef struct Scene {
	
	MeshManager* mms;
	Emitter* emitters;
	
	MapInfo map;
	
	
} Scene;


void scene_init(Scene* sc);


//void initQuadTree(QuadTree* qt, float szX, float szY);
//void qtInsertItem(QuadTree* qt, Renderable* r); 















#endif // __EACSMB_SCENE_H__
