 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"

#include "utilities.h"
#include "map.h"



static GLuint patchVAO;
static GLuint patchVBO;

static GLfloat patchVertices[TERR_PATCH_DIVISOR * TERR_PATCH_DIVISOR * 3 * 4];


static void setQuad(float* f, float x, float y) {
	static float half = .5 / TERR_PATCH_DIVISOR;
	
	f[0] = x - half;
	f[1] = y - half;
	f[2] = 0.0f;
	
	f[3] = x - half;
	f[4] = y + half;
	f[5] = 0.0f;
	
	f[6] = x + half;
	f[7] = y + half;
	f[8] = 0.0f;
	
	f[9] = x + half;
	f[10] = y - half;
	f[11] = 0.0f;
}

void initTerrain() {
	
	
// 	GLint MaxPatchVertices = 0;
// 	glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
// 	
// 	printf("Max supported patch vertices %d\n", MaxPatchVertices);
// 	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	
	int patchCnt = TERR_PATCH_DIVISOR * TERR_PATCH_DIVISOR;
	
	float side = 1 / TERR_PATCH_DIVISOR;
	
// 	GLuint tp_ul = glGetUniformLocation(textProg->id, "mProj");
// 	GLuint tm_ul = glGetUniformLocation(textProg->id, "mModel");
// 	GLuint ts_ul = glGetUniformLocation(textProg->id, "fontTex");
	
	

	int ix, iy;
	for(iy = 0; iy < patchCnt; iy++) {
		for(ix = 0; ix < patchCnt; ix++) {
		
			setQuad(patchVertices + ((ix + (iy * TERR_PATCH_DIVISOR)) * 12), ix, iy);
		}
	}
	
	

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	glGenVertexArrays(1, &patchVAO);
	glBindVertexArray(patchVAO);
	
	glGenBuffers(1, &patchVBO);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(patchVertices), patchVertices, GL_STATIC_DRAW);
	glerr("buffering data");
// 	glBindVertexArray(vbo);
	

	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glerr("vertex attrib ptr");

	
	
	
}




MapBlock* allocMapBlock(size_t stride, int w, int h) {
	
	
	MapBlock* b = malloc(sizeof(MapBlock));
	
	b->data[0] = malloc(w * h * stride * 2);
	b->data[1] = b->data[0] + (w * h * stride);
	
	b->stride = stride;
	b->width = w;
	b->height = h;
	
	b->near[0][0] = NULL;
	b->near[0][1] = NULL;
	b->near[1][0] = NULL;
	b->near[1][1] = NULL;
	
	b->description = NULL;
	
	return b;
}



TerrainBlock* allocTerrainBlock(int cx, int cy) {
	
	TerrainBlock* tb;
	
	// clear for now
	tb = calloc(sizeof(TerrainBlock), 1);
	
	tb->cx = cx;
	tb->cy = cy;
	tb->box.min.x = cx - (TERR_BLOCK_SZ / 2);
	tb->box.max.x = cx + (TERR_BLOCK_SZ / 2);
	tb->box.min.y = cy - (TERR_BLOCK_SZ / 2);
	tb->box.max.y = cy + (TERR_BLOCK_SZ / 2);
	
	tb->tex = 0;
	
}



void updateTerrainTexture(TerrainBlock* tb) {
	
	glGenTextures(1, &tb->tex);
	glBindTexture(GL_TEXTURE_2D, tb->tex);
	
	// we do want mipmaps
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	
	glTexImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		GL_R32F, // internalformat
		TERR_TEX_SZ,
		TERR_TEX_SZ,
		0,  // border
		GL_RED,  // format
		GL_FLOAT, // input type
		tb->zs);
	
}



void drawTerrainBlock(TerrainBlock* tb) {
	
	
	glerr("pre vao bind");
	glBindVertexArray(patchVAO);
	glerr("vao bind");
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
	glDrawArrays(GL_PATCHES, 0, 4);
	glerr("drawing");
	
	
}





/* complicated premature optimization below

static MapNode* allocMapNode(MapNode* parent, int ix, int iy);




void printMapMemoryStats() {
	

	printf("MapBlock size: %d\n", (int)sizeof(MapBlock));
	printf("MapNode size: %d\n", (int)sizeof(MapNode));
	
}



MapNode* allocRootMapNode(short stride) {
	
	MapNode* mn;
	
	
	mn = (MapNode*)malloc(sizeof(MapNode));
	
	// root node stuff
	mn->level = 0;
	mn->x = 0.0;
	mn->y = 0.0;
	mn->parent = NULL;
	
	mn->dataStride = stride;
	
	mn->kids[0][0] = allocMapNode(mn, 0, 0);
	mn->kids[0][1] = allocMapNode(mn, 0, 1);
	mn->kids[1][0] = allocMapNode(mn, 1, 0);
	mn->kids[1][1] = allocMapNode(mn, 1, 1);
	
	mn->usage = 0x00;
	
	return mn;
}





static MapNode* allocMapNode(MapNode* parent, int ix, int iy) {
	
	MapNode* mn;
	
	mn = (MapNode*)malloc(sizeof(MapNode));
	
	// root node stuff
	mn->level = parent->level + 1;
	mn->x = ix;
	mn->y = iy;
	mn->parent = parent;
	mn->kids[0][0] = NULL;
	mn->kids[0][1] = NULL;
	mn->kids[1][0] = NULL;
	mn->kids[1][1] = NULL;
	
	mn->usage = 0;
	
	return mn;
}
	
	
void allocBlockData(MapNode* mn, char ix, char iy) {
	
	MapBlock* mb;
	
	mb = (MapBlock*)malloc(sizeof(MapBlock));
	
	mb->data = calloc(mn->dataStride * MAP_SZ * MAP_SZ, 1);
	mb->counter = 0;
	
	// TODO: fill in location;
	// TODO: fill in neighbors
	
	mn->usage |= MAP_IDX_MASK(ix,iy);
	mn->blocks[ix][iy] = mb;
	
}



*/



