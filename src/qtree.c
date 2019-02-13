

#include "world.h"



static QuadTreeNode* QTNode_alloc(QuadTree* qt, QuadTreeNode* parent, char x, char y);
static int QuadTreeNode_addSII(QuadTreeNode* n, SceneItemInfo* siNew);
static int QuadTreeNode_remSII(QuadTreeNode* n, SceneItemInfo* siDead);
static int QuadTreeNode_insert(QuadTree* qt, QuadTreeNode* n, SceneItemInfo* siNew);
static int QuadTreeNode_purge(QuadTree* qt, QuadTreeNode* n, SceneItemInfo* siDead);
static SceneItemInfo* QuadTreeNode_findFirst(QuadTree* qt, QuadTreeNode* n, Vector2 pt);
static int QuadTreeNode_findAll(QuadTree* qt, QuadTreeNode* n, Vector2 pt, QuadTreeFindFn fn, void* data);
static int QuadTreeNode_findAllArea(QuadTree* qt, QuadTreeNode* n, AABB2 aabb, QuadTreeFindFn fn, void* data);
// int QuadTreeNode_split(QuadTree* qt, QuadTreeNode* n);



void QuadTree_init(QuadTree* qt, AABB2 bounds) {
	
	*qt = (QuadTree){};
	
	qt->maxLevels = 5;
	qt->nodeMaxCount = 30;
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
void QuadTree_purge(QuadTree* qt, SceneItemInfo* siDead) {
	QuadTreeNode_purge(qt, qt->root, siDead);
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
// TODO: add a perf timer on this to see if a better algothms should be used
// returns 1 if a pointer was removed, 0 otherwise
static int QuadTreeNode_remSII(QuadTreeNode* n, SceneItemInfo* siDead) {
	VEC_EACH(&n->items, sii, si) {
		if(si == siDead) {
			VEC_RM(&n->items, sii);
			return 1;
		}
	}
	
	return 0;
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


static int QuadTreeNode_purge(QuadTree* qt, QuadTreeNode* n, SceneItemInfo* siDead) {
	
	if(boxDisjoint2(&siDead->aabb, &n->aabb)) {
		return 0;
	}
	
	// purge from this node's list
	int c = QuadTreeNode_remSII(n, siDead);
	
	if(!n->kids[0][0]) return c;
	
	// purge from subnodes
	for(int x = 0; x <= 1; x++) {
		for(int y = 0; y <= 1; y++) {
			c += QuadTreeNode_purge(qt, n->kids[x][y], siDead);
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


SceneItemInfo* QuadTree_findFirst(QuadTree* qt, Vector2 pt) {
	return QuadTreeNode_findFirst(qt, qt->root, pt);
}

static SceneItemInfo* QuadTreeNode_findFirst(QuadTree* qt, QuadTreeNode* n, Vector2 pt) {
	if(VEC_LEN(&n->items) == 0) return 0; 
	
	if(!boxContainsPoint2(&n->aabb, &pt)) {
		return NULL;
	}
// 	printf("level %d\n", n->level);
	if(n->kids[0][0]) {
		
		// search kids instead
		for(int x = 0; x <= 1; x++) {
			for(int y = 0; y <= 1; y++) {
				SceneItemInfo* si = QuadTreeNode_findFirst(qt, n->kids[x][y], pt);
				if(si) return si;
			}
		}
		
		// there is no item over the point
		return NULL;
	}
	else {
		// leaf node; search here 
		VEC_EACH(&n->items, sii, si) {
			if(boxContainsPoint2(&si->aabb, &pt)) {
				return si;
			}
		}
	}
	
	
//	printf("\nERROR: unreachable path 2 %s:%d\n\n", __FILE__, __LINE__);
	
	return NULL;
}



void QuadTree_findAll(QuadTree* qt, Vector2 pt, QuadTreeFindFn fn, void* data) {
	QuadTreeNode_findAll(qt, qt->root, pt, fn, data);
}

// return 0 to continue, 1 to stop 
static int QuadTreeNode_findAll(QuadTree* qt, QuadTreeNode* n, Vector2 pt, QuadTreeFindFn fn, void* data) {
	if(VEC_LEN(&n->items) == 0) return 0; 
	
	if(!boxContainsPoint2(&n->aabb, &pt)) {
		return 0;
	}

	if(n->kids[0][0]) {
		
		// search kids instead
		for(int x = 0; x <= 1; x++) {
			for(int y = 0; y <= 1; y++) {
				if(QuadTreeNode_findAll(qt, n->kids[x][y], pt, fn, data)) return 1;
			}
		}
		
		// keep going
		return 0;
	}
	else {
		// leaf node; search here 
		VEC_EACH(&n->items, sii, si) {
			if(boxContainsPoint2(&si->aabb, &pt)) {
				if(fn(si, data)) return 1;
			}
		}
	}
	
	return 0;
}

void QuadTree_findAllArea(QuadTree* qt, AABB2 aabb, QuadTreeFindFn fn, void* data) {
	QuadTreeNode_findAllArea(qt, qt->root, aabb, fn, data);
}

// return 0 to continue, 1 to stop 
static int QuadTreeNode_findAllArea(QuadTree* qt, QuadTreeNode* n, AABB2 aabb, QuadTreeFindFn fn, void* data) {
	if(VEC_LEN(&n->items) == 0) return 0; 
	
	if(boxDisjoint2(&n->aabb, &aabb)) {
		return 0;
	}

	if(n->kids[0][0]) {
		
		// search kids instead
		for(int x = 0; x <= 1; x++) {
			for(int y = 0; y <= 1; y++) {
				if(QuadTreeNode_findAllArea(qt, n->kids[x][y], aabb, fn, data)) return 1;
			}
		}
		
		// keep going
		return 0;
	}
	else {
		// leaf node; search here 
		VEC_EACH(&n->items, sii, si) {
			if(!boxDisjoint2(&si->aabb, &aabb)) {
				if(fn(si, data)) return 1;
			}
		}
	}
	
	return 0;
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
	
	
	// render items on the leaf node
	static Vector colors[] = {
		{1,.2, .5},
		{.1,.5, .5},
		{.2,.6, .8},
		{.5,.1, .8},
		{.1,.9, .1},
		{.1,.6, .3},
	};
	int r = (int)(n->aabb.min.x + n->aabb.min.y) % 6;
	
	if(!n->kids[0][0]) {
		VEC_EACH(&n->items, sii, si) {
			renderDebugBox(1,
				colors[r],
				(Vector){si->aabb.min.x, si->aabb.min.y, hmin+5},
				(Vector){si->aabb.max.x, si->aabb.max.y, hmin+10},
				pfp);
		}
	}
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






