#ifndef __scene_h__
#define __scene_h__




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




typedef struct Scene {
	
	int totalItems;
	
	QuadTree qt;
	
} Scene;





void initQuadTree(QuadTree* qt, float szX, float szY);
void qtInsertItem(QuadTree* qt, Renderable* r); 















#endif // __scene_h__