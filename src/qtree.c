

#include "world.h"



static QuadTreeNode* QTNode_alloc(QuadTree* qt, QuadTreeNode* parent, char x, char y);
static int QuadTreeNode_addSII(QuadTreeNode* n, SceneItemInfo* siNew);
static int QuadTreeNode_insert(QuadTree* qt, QuadTreeNode* n, SceneItemInfo* siNew);
 int QuadTreeNode_split(QuadTree* qt, QuadTreeNode* n);


void QuadTree_init(QuadTree* qt, AABB2 bounds) {
	
	*qt = (QuadTree){};
	
	qt->maxLevels = 4;
	qt->nodeMaxCount = 100;
	qt->nodeMinCount = 1;
	
	MemPool_init(&qt->siPool, sizeof(SceneItemInfo), 1024*1024);
	
	qt->root = QTNode_alloc(qt, NULL, 0, 0);
	qt->root->aabb = bounds;
}


SceneItemInfo* QuadTree_allocSceneItem(QuadTree* qt) {
	SceneItemInfo* si;
	si = MemPool_calloc(&qt->siPool);
	return si;
}



void QuadTree_insert(QuadTree* qt, SceneItemInfo* siNew) {
	QuadTreeNode_insert(qt, qt->root, siNew);
}




static QuadTreeNode* QTNode_alloc(QuadTree* qt, QuadTreeNode* parent, char x, char y) {
	QuadTreeNode* n;
	pcalloc(n);
	
	if(parent) {
		n->parent = parent;
		n->level = parent->level + 1;
		
		boxQuadrant2(&n->parent->aabb, x, y, &n->aabb);
	}
	
	VEC_INIT(&n->items);
	
	return n;
}


// TODO: add a perf timer on this to see if a better algothms should be used
// returns 1 if a new pointer was added, 0 otherwise
static int QuadTreeNode_addSII(QuadTreeNode* n, SceneItemInfo* siNew) {
	VEC_EACH(&n->items, sii, si) {
		if(si == siNew) return 0;
	}
	
	VEC_PUSH(&n->items, siNew);
	return 1;
}


static int QuadTreeNode_insert(QuadTree* qt, QuadTreeNode* n, SceneItemInfo* siNew) {
	
	if(boxDisjoint2(&siNew->aabb, &n->aabb)) {
		return 0;
	}
	
	// insert into this node's list
	int c = QuadTreeNode_addSII(n, siNew);
	
	// limit depth
	if(n->level >= qt->maxLevels) return c;
	
	// maybe split the node
	if(!n->kids[0][0]) {
		if(VEC_LEN(&n->items) < qt->nodeMaxCount) return c;
		QuadTreeNode_split(qt, n);
	}
	
	// insert into subnodes
	for(int x = 0; x <= 1; x++) {
		for(int y = 0; y <= 1; y++) {
			c += QuadTreeNode_insert(qt, n->kids[x][y], siNew);
		}
	}
	
	return c;
}



int QuadTreeNode_split(QuadTree* qt, QuadTreeNode* n) {
	
	n->kids[0][0] = QTNode_alloc(qt, n, 0, 0);
	n->kids[0][1] = QTNode_alloc(qt, n, 0, 1);
	n->kids[1][0] = QTNode_alloc(qt, n, 1, 0);
	n->kids[1][1] = QTNode_alloc(qt, n, 1, 1);
	
	VEC_EACH(&n->items, sii, si) {
		 QuadTreeNode_insert(qt, n->kids[0][0], si);
		 QuadTreeNode_insert(qt, n->kids[0][1], si);
		 QuadTreeNode_insert(qt, n->kids[1][0], si);
		 QuadTreeNode_insert(qt, n->kids[1][1], si);
	}
	
	qt->totalNodes += 4;
}






static void renderDebugBox(float thickness, Vector color, Vector min, Vector max, PassFrameParams* pfp);

static void QuadTreeNode_renderDebugVolumes(QuadTreeNode* n, float hmin, float hmax, PassFrameParams* pfp) {
	if(!n) return;
	
	renderDebugBox(1,
		(Vector){1,0,0},
		(Vector){n->aabb.min.x, n->aabb.min.y, hmin},
		(Vector){n->aabb.max.x, n->aabb.max.y, hmax},
		pfp);
	
	QuadTreeNode_renderDebugVolumes(n->kids[0][0], hmin+1, hmax-2, pfp);
	QuadTreeNode_renderDebugVolumes(n->kids[0][1], hmin+1, hmax-2, pfp);
	QuadTreeNode_renderDebugVolumes(n->kids[1][0], hmin+1, hmax-2, pfp);
	QuadTreeNode_renderDebugVolumes(n->kids[1][1], hmin+1, hmax-2, pfp);
}


void QuadTree_renderDebugVolumes(QuadTree* qt, PassFrameParams* pfp) {
	QuadTreeNode_renderDebugVolumes(qt->root, 20, 70, pfp);
	
}

static void renderDebugBox(float thickness, Vector color, Vector min, Vector max, PassFrameParams* pfp) {
	
	static GLuint vbo = 0;
	static GLuint vao = 0;
	static ShaderProgram* prog = NULL;
	
	static GLuint wp_ul, wv_ul, min_ul, max_ul, color_ul;
	
	if(prog == NULL) {
		prog = loadCombinedProgram("debug_box");
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		
		glEnableVertexAttribArray(0);
		glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);
		
		static float box[] = {
			0,0,0, 0,0,1,
			0,0,1, 0,1,1,
			0,1,1, 0,1,0,
			0,1,0, 0,0,0,
			
			1,0,0, 1,0,1,
			1,0,1, 1,1,1,
			1,1,1, 1,1,0,
			1,1,0, 1,0,0,
			
			1,0,0, 0,0,0,
			1,0,1, 0,0,1,
			1,1,1, 0,1,1,
			1,1,0, 0,1,0,
		};
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_STATIC_DRAW);
		
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glexit("");
		
// 		wp_ul = glGetUniformLocation(prog->id, "mWorldProj");
		wv_ul = glGetUniformLocation(prog->id, "mWorldView");
		wp_ul = glGetUniformLocation(prog->id, "mViewProj");
		min_ul = glGetUniformLocation(prog->id, "pMin");
		max_ul = glGetUniformLocation(prog->id, "pMax");
		color_ul = glGetUniformLocation(prog->id, "color");
		
		// fixed location of depth texture
		glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sDepth"), 2);
		
		glexit("");
	}
	
	
	
	glUseProgram(prog->id);
// 	glUniformMatrix4fv(wp_ul, 1, GL_FALSE, &pfp->dp->mWorldProj);
	glUniformMatrix4fv(wv_ul, 1, GL_FALSE, &pfp->dp->mWorldView->m);
	glUniformMatrix4fv(wp_ul, 1, GL_FALSE, &pfp->dp->mViewProj->m);
	glUniform3f(min_ul, min.x, min.y, min.z);
	glUniform3f(max_ul, max.x, max.y, max.z);
	glUniform3fv(color_ul, 1, &color);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glexit("debug box vbo");
	
// 	glDrawArrays(GL_LINES, 0, 12 * 2);
	glLineWidth(thickness);
	glDrawArrays(GL_LINES, 0, 12 * 2);
	glexit("debug box draw");
	
}













