 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"
#include "map.h"



static GLuint patchVAO;
static GLuint patchVBO;
static GLuint proj_ul, view_ul, model_ul, heightmap_ul, winsize_ul, basetex_ul; 
static totalPatches;
Texture* cnoise;

static TerrainPatchVertex* patchVertices;

ShaderProgram* terrProg;

GLuint MaxPatchVertices;
GLuint MaxTessGenLevel;


void initTerrain() {
	
	
 	GLint MaxPatchVertices = 0;
 	glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
 	if(MaxPatchVertices < 4) {
		fprintf(stderr, "FATAL: GL_MAX_PATCH_VERTICES is too low: %d. Minimum required value is 4.\n", MaxPatchVertices);
		exit(3);
	};
	
	glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &MaxTessGenLevel);
	printf("GL_MAX_TESS_GEN_LEVEL: %d\n", MaxTessGenLevel);
	if(MaxTessGenLevel < 32) {
		fprintf(stderr, "FATAL: GL_MAX_TESS_GEN_LEVEL is too low: %d. Minimum required value is 32.\n", MaxTessGenLevel);
		exit(3);
	};
	
	
	cnoise = loadBitmapTexture("./assets/textures/colornoise.png");
	
	
	glerr("clearing before terrain program load");
	terrProg = loadProgram("terrain", "terrain", NULL, "terrain", "terrain");
	
	
	model_ul = glGetUniformLocation(terrProg->id, "mModel");
	glerr("terrain uniform loc Model");
	view_ul = glGetUniformLocation(terrProg->id, "mView");
	glerr("terrain uniform loc View");
	proj_ul = glGetUniformLocation(terrProg->id, "mProj");
	glerr("terrain uniform loc Projection");
	heightmap_ul = glGetUniformLocation(terrProg->id, "sHeightMap");
	glerr("terrain uniform loc hm");
	basetex_ul = glGetUniformLocation(terrProg->id, "sBaseTex");
	glerr("terrain uniform loc tex");
	winsize_ul = glGetUniformLocation(terrProg->id, "winSize");
	glerr("terrain uniform loc ws");

	
	
	// in one dimension
	totalPatches = TERR_TEX_SZ / MaxTessGenLevel; //wholePatches + (fracPatchSize > 0 ? 1 : 0);
		
	int patchCnt = (totalPatches * totalPatches);
	patchVertices = malloc(sizeof(TerrainPatchVertex) * 4 * patchCnt);
	
	
	float sideUnit = 1.0 / TERR_TEX_SZ; // size of a square, the smallest tessellation level
	float wpSide = sideUnit * MaxTessGenLevel; // total length of a whole patch side
	
	
	TerrainPatchVertex* pv = patchVertices;
	
	int ix, iy;
	for(iy = 0; iy < totalPatches; iy++) {
		for(ix = 0; ix < totalPatches; ix++) {
			//printf("pv: %d\n", pv);
			int tlX = MaxTessGenLevel;
			int tlY = MaxTessGenLevel;
			
			
			pv->x = (ix * wpSide);
			pv->y = (iy * wpSide);
			pv->z = 0;
			pv->hmU = (ix * MaxTessGenLevel * sideUnit);
			pv->hmV = (iy * MaxTessGenLevel * sideUnit);
			pv->divX = ix * sideUnit;
			pv->divY = iy * sideUnit;
			pv++;
			
// 			printf("divx: %d\n", iy * tlY);
			
	//		printf("u %f, v %f\n",
	//			baseTexUV + (ix * MaxTessGenLevel * sideUnit),
	//			baseTexUV + (iy * MaxTessGenLevel * sideUnit)
	//		);

			pv->x = (ix * wpSide);
			pv->y = ((iy+1) * wpSide);
			pv->z = 0;
			pv->hmU = (ix * MaxTessGenLevel * sideUnit);
			pv->hmV = ((iy+1) * MaxTessGenLevel * sideUnit);
			pv->divX = ix * sideUnit;
			pv->divY = (iy+1) * sideUnit;
			pv++;

			pv->x = ((ix+1) * wpSide);
			pv->y = ((iy+1) * wpSide);
			pv->z = 0;
			pv->hmU = ((ix+1) * MaxTessGenLevel * sideUnit);
			pv->hmV = ((iy+1) * MaxTessGenLevel * sideUnit);
			pv->divX = (ix+1) * sideUnit;
			pv->divY = (iy+1) * sideUnit;
			pv++;

			pv->x = ((ix+1) * wpSide);
			pv->y = (iy * wpSide);
			pv->z = 0;
			pv->hmU = ((ix+1) * MaxTessGenLevel * sideUnit);
			pv->hmV = (iy * MaxTessGenLevel * sideUnit);
			pv->divX = (ix+1) * sideUnit;
			pv->divY = iy * sideUnit;
			pv++;
		}
	}
	
	
	
	
	
// 	GLuint tp_ul = glGetUniformLocation(textProg->id, "mProj");
// 	GLuint tm_ul = glGetUniformLocation(textProg->id, "mModel");
// 	GLuint ts_ul = glGetUniformLocation(textProg->id, "fontTex");

	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	glGenVertexArrays(1, &patchVAO);
	glBindVertexArray(patchVAO);
	
	glGenBuffers(1, &patchVBO);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);

			// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainPatchVertex), 0);
	
	// heightmap texel coordinates
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainPatchVertex), (void*)offsetof(TerrainPatchVertex, hmU));
	
	// data for TCS output divisions
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainPatchVertex), (void*)offsetof(TerrainPatchVertex, divX));
	glexit("terrain vertex attrib calls");
	
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(TerrainPatchVertex) * 4 * patchCnt, patchVertices, GL_STATIC_DRAW);
	glexit("buffering terrain patch vertex data");
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
	
	int x, y;
	for(y = 0; y < TERR_TEX_SZ ; y++) {
		for(x = 0; x < TERR_TEX_SZ ; x++) {
			tb->zs[x + (y * TERR_TEX_SZ)] = sin(x * .01);
		}
	}
	
	
	return tb;
}



void updateTerrainTexture(TerrainBlock* tb) {
	
	if(!tb->tex) {
		glGenTextures(1, &tb->tex);
	}
	glBindTexture(GL_TEXTURE_2D, tb->tex);
	
	// we do want mipmaps
	//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	
	// need to switch to nearest later on 
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
	glerr("failed to load terrain heightmap");
}



void drawTerrainBlock(TerrainBlock* tb, Matrix* mModel, Matrix* mView, Matrix* mProj) {
	
	glUseProgram(terrProg->id);
	glexit("using terrain program");
	
	glUniformMatrix4fv(model_ul, 1, GL_FALSE, mModel->m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, mView->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, mProj->m);
	glexit("terrain matrix uniforms");
	
	
	
	glUniform2f(winsize_ul, 600, 600);
	
	
	
	glActiveTexture(GL_TEXTURE0);
	
	glexit("active texture");
	
	glBindTexture(GL_TEXTURE_2D, tb->tex);
	glexit("bind hm texture");
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cnoise->tex_id);
	glexit("bind base texture");

	glUniform1i(heightmap_ul, 0);
	glexit("hm sampler uniform");
	glUniform1i(basetex_ul, 1);
	glexit("base tex sampler uniform");

	
	glerr("pre vao bind");
	glBindVertexArray(patchVAO);
	glerr("vao bind");
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
	glDrawArrays(GL_PATCHES, 0, totalPatches * totalPatches * 4);
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



