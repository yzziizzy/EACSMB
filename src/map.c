 
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
#include "perlin.h"



static GLuint patchVAO;
static GLuint patchVBO;
static GLuint proj_ul, view_ul, model_ul, heightmap_ul, winsize_ul, basetex_ul;
static GLuint proj_d_ul, view_d_ul, model_d_ul, heightmap_d_ul;
static GLuint map_ul, zoneColors_ul;
static totalPatches;
Texture* cnoise;

static TerrainPatchVertex* patchVertices;

ShaderProgram* terrProg;
ShaderProgram* terrDepthProg;

GLuint MaxPatchVertices;
GLuint MaxTessGenLevel;

static MatrixStack model;

char* tmpSavePath;
char* tmpSaveName = "EACSMB-map-cache";


void initMap(MapInfo* mi) {
	
	char* tmpDir;
	
	
	tmpDir = getenv("TMPDIR");
	
	if(!tmpDir) tmpDir = P_tmpdir;
	
	tmpSavePath = malloc(strlen(tmpDir) + strlen(tmpSaveName) + 2);
	
	strcpy(tmpSavePath, tmpDir);
	strcat(tmpSavePath, "/");
	strcat(tmpSavePath, tmpSaveName);
	
	
	
	initTerrain();
	
	// HACK: should probably be scaled by data from mapinfo
	msAlloc(4, &model);
	msIdent(&model);
	msScale3f(TERR_TEX_SZ,TERR_TEX_SZ,TERR_TEX_SZ, &model);
	msPush(&model);
	
	mi->blocksSz = 32 * 32;
	mi->blocks = malloc(sizeof(MapBlock*) * mi->blocksSz);
	

	mi->originMB = allocMapBlock(0, 0);
	mi->blocks[0] = mi->originMB;
	mi->blocksLen++;
	
	mi->blocks[mi->blocksLen] = allocMapBlock(1, 0);
	mi->blocks[mi->blocksLen]->n_xm = mi->blocks[0];
	mi->blocks[0]->n_xp = mi->blocks[mi->blocksLen];
	mi->blocksLen++;
	
	
	
	// zone stuff
	memset(&mi->zoneColors, 0, 256);
	
	// TODO: load zone colors
	mi->zoneColors[0] = 0x00000000;
	mi->zoneColors[1] = 0x0000ff00;
	mi->zoneColors[2] = 0x00ff0000;
	mi->zoneColors[3] = 0x0000FFFF;
	
	glGenTextures(1, &mi->zoneColorTex);
	glBindTexture(GL_TEXTURE_1D, mi->zoneColorTex);
	
// 	glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP, GL_FALSE);
	
	// need to switch to nearest later on
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

// 	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage1D(GL_TEXTURE_1D, // target
		0,  // level, 0 = base, no minimap,
		GL_RGBA8, // internalformat
		256,
		0,  // border
		GL_RGBA,  // format
		GL_UNSIGNED_BYTE, // input type
		mi->zoneColors);
	
	glerr("zone color map");
	
	

}



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
	
	
	cnoise = loadBitmapTexture("./assets/textures/grass_texture-256.png");
	
	terrProg = loadCombinedProgram("terrain");
	terrDepthProg = loadCombinedProgram("terrainDepth");
	
	model_ul = glGetUniformLocation(terrProg->id, "mModel");
	view_ul = glGetUniformLocation(terrProg->id, "mView");
	proj_ul = glGetUniformLocation(terrProg->id, "mProj");

	heightmap_ul = glGetUniformLocation(terrProg->id, "sHeightMap");
	basetex_ul = glGetUniformLocation(terrProg->id, "sBaseTex");
	winsize_ul = glGetUniformLocation(terrProg->id, "winSize");
	zoneColors_ul = glGetUniformLocation(terrProg->id, "sZoneColors");
	map_ul = glGetUniformLocation(terrProg->id, "sMap");



	model_d_ul = glGetUniformLocation(terrDepthProg->id, "mModel");
	view_d_ul = glGetUniformLocation(terrDepthProg->id, "mView");
	proj_d_ul = glGetUniformLocation(terrDepthProg->id, "mProj");
	heightmap_d_ul = glGetUniformLocation(terrDepthProg->id, "sHeightMap");

	
	
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
			pv->divX = ix * MaxTessGenLevel;
			pv->divY = iy * MaxTessGenLevel;
			pv++;
			
			pv->x = (ix * wpSide);
			pv->y = ((iy+1) * wpSide);
			pv->z = 0;
			pv->hmU = (ix * MaxTessGenLevel * sideUnit);
			pv->hmV = ((iy+1) * MaxTessGenLevel * sideUnit);
			pv->divX = ix * MaxTessGenLevel;
			pv->divY = (iy+1) * MaxTessGenLevel;
			pv++;

			pv->x = ((ix+1) * wpSide);
			pv->y = ((iy+1) * wpSide);
			pv->z = 0;
			pv->hmU = ((ix+1) * MaxTessGenLevel * sideUnit);
			pv->hmV = ((iy+1) * MaxTessGenLevel * sideUnit);
			pv->divX = (ix+1) * MaxTessGenLevel;
			pv->divY = (iy+1) * MaxTessGenLevel;
			pv++;

			pv->x = ((ix+1) * wpSide);
			pv->y = (iy * wpSide);
			pv->z = 0;
			pv->hmU = ((ix+1) * MaxTessGenLevel * sideUnit);
			pv->hmV = (iy * MaxTessGenLevel * sideUnit);
			pv->divX = (ix+1) * MaxTessGenLevel;
			pv->divY = iy * MaxTessGenLevel;
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




MapBlock* findMapBlock(MapInfo* mi, int bix, int biy) {

}


MapBlockTreeLeaf* allocMapBlockTreeLeaf(int minx, int miny) {
	MapBlockTreeLeaf* p;
	p = malloc(sizeof(MapBlockTreeLeaf));
	p->minx = minx;
	p->miny = miny;
	return p;
}


MapBlockLeaf* spawnMapBlockLeaf(Mapinfo*mi, int llbix, llbiy) {
	int x, y;
	MapBlockLeaf* mbl;
	
	mbl = allocMapBlockLeaf(mi, llbix, llbix);
	
	
	for(y = 0; y < 16; y++) {
		for(x = 0; x < 16; x++) {
			mbl->c[x][y] = spawnMapBlock(mi, mbl, llbix + x, llbiy + y);
		}
	}
	
	return mbl;
}

MapBlock* spawnMapBlock(MapInfo* mi, int bix, int biy) {
	
	MapBlock* mb;
	
	mb = allocMapBlock(bix, biy);
	initTerrainBlock(mb, bix, biy);
	
	return mb;
}


MapBlock* allocMapBlock(int bix, int biy) {
	
	
	MapBlock* b = calloc(sizeof(MapBlock), 1);
	b->bix = bix;
	b->biy = biy;
	
	return b;
}



void initTerrainBlock(MapBlock* mb, int cx, int cy) {
	
	
	TerrainBlock* tb = &mb->tb;
	FILE* f;
	
	float offx = cx * TERR_BLOCK_SZ;
	float offy = cy * TERR_BLOCK_SZ;
	
	
	tb->cx = cx;
	tb->cy = cy;
	
	
	
	// BUG: probably wrong
	tb->box.min.x = cx - (TERR_BLOCK_SZ / 2);
	tb->box.max.x = cx + (TERR_BLOCK_SZ / 2);
	tb->box.min.y = cy - (TERR_BLOCK_SZ / 2);
	tb->box.max.y = cy + (TERR_BLOCK_SZ / 2);
	
	tb->tex = 0;
	
	f = fopen(tmpSavePath, "rb");
	
	if(0 && f) {
		// load data
		printf("Loading saved terrain from %s\n", tmpSavePath);
		fread(tb->zs, sizeof(float), TERR_TEX_SZ * TERR_TEX_SZ, f);
		
		fclose(f);
	}
	else {
		// generate new data and save it
		printf("Generating new terrain... ");
		int x, y;
		for(y = 0; y < TERR_TEX_SZ ; y++) {
			for(x = 0; x < TERR_TEX_SZ ; x++) {
				//tb->zs[x + (y * TERR_TEX_SZ)] = sin(x * .1) * .1;
				float f = PerlinNoise_2D((offx + x) / 128.0, (offy + y) / 128.0, .1, 6); // slow-ass function, disable except for noise testing
	// 			printf("[%d,%d] %f\n", x,y,f);
				tb->zs[x + (y * TERR_TEX_SZ)] = fabs(1-f) * 100;
			}
		}
		
		/*
		printf("done.\nSaving terrain to %s\n", tmpSavePath);
		f = fopen(tmpSavePath, "wb");
		if(!f) return tb;
		
		fwrite(tb->zs, sizeof(float), TERR_TEX_SZ * TERR_TEX_SZ, f);
		
		fclose(f);
		*/
	}
	
	
}




void checkMapDirty(MapInfo* mi) { /*
	if(mi->mb->dirtyZone || mi->mb->dirtySurface) {
		updateMapTextures(mi->mb);
		mi->mb->dirtyZone = 0;
		mi->mb->dirtySurface = 0;
	}
	
	if(mi->tb->dirty) {
		updateTerrainTexture(&mi->originMB.tb);
		mi->tb->dirty = 0;
	}
	*/
}


void updateMapTextures(MapInfo* mi) {
	if(!mi->tex) {
		
		glGenTextures(1, &mi->tex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, mi->tex);
		glexit("failed to create map textures b");
		
// 		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_FALSE);
		glexit("failed to create map textures c");

		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glexit("failed to create map textures a");
		
		// squash the data in
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		glTexStorage3D(GL_TEXTURE_2D_ARRAY,
			1,  // mips, flat
			GL_R8UI,
			MAP_TEX_SZ, MAP_TEX_SZ,
			32); // layers
		
		glexit("failed to create map textures");
	}
	else {
		glBindTexture(GL_TEXTURE_2D_ARRAY, mi->tex);
	}
	
	/*
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
		0,  // mip level, 0 = base, no mipmap,
		0, 0, 0,// offset
		MAP_TEX_SZ,
		MAP_TEX_SZ,
		1,
		GL_RED_INTEGER,  // format
		GL_UNSIGNED_BYTE, // input type
		mb->zones);
	*/
	
	int i;
	for(i = 0; i < 32; i++) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
			0,  // mip level, 0 = base, no mipmap,
			0, 0, i,// offset
			MAP_TEX_SZ,
			MAP_TEX_SZ,
			1,
			GL_RED_INTEGER,  // format
			GL_UNSIGNED_BYTE, // input type
			mi->texIndexMap[i]->surface);
	}
	
	
	glerr("failed to update map tex info");
}


void updateTerrainTexture(MapInfo* mi) {
	if(!mi->terrainTex) {
		
		glGenTextures(1, &mi->terrainTex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, mi->terrainTex);
		glexit("failed to create map textures b");
		
// 		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, GL_FALSE);
		glexit("failed to create map textures c");

		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glexit("failed to create map textures a");
		
		// squash the data in
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		glTexStorage3D(GL_TEXTURE_2D_ARRAY,
			1,  // mips, flat
			GL_R32F,
			MAP_TEX_SZ, MAP_TEX_SZ,
			32); // layers
		
		glexit("failed to create map textures");
	}
	else {
		glBindTexture(GL_TEXTURE_2D_ARRAY, mi->terrainTex);
	}
	
	/*
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
		0,  // mip level, 0 = base, no mipmap,
		0, 0, 0,// offset
		MAP_TEX_SZ,
		MAP_TEX_SZ,
		1,
		GL_RED_INTEGER,  // format
		GL_UNSIGNED_BYTE, // input type
		mb->zones);
	*/
	
	int i;
	for(i = 0; i < 32; i++) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
			0,  // mip level, 0 = base, no mipmap,
			0, 0, i,// offset
			MAP_TEX_SZ,
			MAP_TEX_SZ,
			1,
			GL_RED,  // format
			GL_FLOAT, // input type
			mi->texIndexMap[i]->zs);
	}
	
	
	glerr("updating terrain tex info");
}


/*
void updateTerrainTexture(MapInfo* mi) {
	
	if(!tb->tex) {
		glGenTextures(1, &tb->tex);
		printf("tex num: %d \n", tb->tex);
		glBindTexture(GL_TEXTURE_2D, tb->tex);
		
		
	}
	else {
		glBindTexture(GL_TEXTURE_2D, tb->tex);
	}
	
	
	if(tb->tex) {
		glBindTexture(GL_TEXTURE_2D, tb->tex);
		
		glTexSubImage2D(GL_TEXTURE_2D, // target
			0,  // level, 0 = base, no minimap,
			0, 0, // offset
			TERR_TEX_SZ,
			TERR_TEX_SZ,
			GL_RED,  // format
			GL_FLOAT, // input type
			tb->zs);
		glerr("updating terrain texture");
		
		return;
	}
	

	
// 	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	
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


*/

void drawTerrainDepth(MapInfo* mi, Matrix* mView, Matrix* mProj, Vector2* viewWH) {
	
	glUseProgram(terrDepthProg->id);
	glEnable(GL_DEPTH_TEST);
	
	drawTerrainBlockDepth(mi, &mi->originMB->tb, msGetTop(&model), mView, mProj, viewWH);
}



void drawTerrain(MapInfo* mi, Matrix* mView, Matrix* mProj, Vector2* cursor, Vector2* viewWH) {
	
	int i;
	
	glUseProgram(terrProg->id);
	glEnable(GL_DEPTH_TEST);
	
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, mView->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, mProj->m);
	
	glUniform2f(winsize_ul, viewWH->x, viewWH->y);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tb->tex);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cnoise->tex_id);
	
// 	glActiveTexture(GL_TEXTURE3);
// 	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->originMB->tex);
	
	glUniform1i(heightmap_ul, 0);
	glUniform1i(basetex_ul, 1);
	
	glUniform2f(glGetUniformLocation(terrProg->id, "cursorPos"), cursor->x, cursor->y);
	glBindVertexArray(patchVAO);
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
	
	for(i = 0; i < 32; i++) {
		if(!mi->texIndexMap[i]) continue;
		
		drawTerrainBlock(mi, i, msGetTop(&model), mView, mProj, cursor, viewWH);
		
	}
}


void drawTerrainBlock(MapInfo* mi, TerrainBlock* tb, Matrix* mModel, Matrix* mView, Matrix* mProj, Vector2* cursor, Vector2* viewWH) {
	
	
	// move the model matrix around

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, mModel->m);


	glDrawArrays(GL_PATCHES, 0, totalPatches * totalPatches * 4);
}




void drawTerrainBlockDepth(MapInfo* mi, TerrainBlock* tb, Matrix* mModel, Matrix* mView, Matrix* mProj, Vector2* viewWH) {
	
	

	
	glUniformMatrix4fv(model_d_ul, 1, GL_FALSE, mModel->m);
	glUniformMatrix4fv(view_d_ul, 1, GL_FALSE, mView->m);
	glUniformMatrix4fv(proj_d_ul, 1, GL_FALSE, mProj->m);
	
	glUniform2f(winsize_ul, viewWH->x, viewWH->y);
	
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tb->tex);
	glUniform1i(heightmap_d_ul, 0);

	glBindVertexArray(patchVAO);
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
	glDrawArrays(GL_PATCHES, 0, totalPatches * totalPatches * 4);
}



// interpreted as a vertical projection of the quad
void genDecalMesh(Quad* q, float zOffset) {
	int x, y, mx, my;
	Vector2 min, max;
	
	min.x = fmin(fmin(q->v[0].x, q->v[1].x), fmin(q->v[0].x, q->v[1].x));
	min.y = fmin(fmin(q->v[0].y, q->v[1].y), fmin(q->v[0].y, q->v[1].y));
	max.x = fmax(fmax(q->v[0].x, q->v[1].x), fmax(q->v[0].x, q->v[1].x));
	max.y = fmax(fmax(q->v[0].y, q->v[1].y), fmax(q->v[0].y, q->v[1].y));
	
	mx = ceil(max.x);
	my = ceil(max.y);
	
	// step 1: duplicate a mesh of all tiles the quad overlaps
	for(y = floor(min.y); y <= my; y++) {
		for(x = floor(min.y); x <= mx; x++) {
			
			
			
			
		}
	}
	
	
	// step 2: slice off any parts outside the quad
	
	
	
	
}



// coordinates are in game tiles.
void areaStats(TerrainBlock* tb, int x1, int y1, int x2, int y2, AreaStats* ass) {
	int x, y, xmin, ymin, xmax, ymax, count;
	float min, max, total;
	
	xmin = MIN(x1, x2);
	xmax = MAX(x1, x2) + 1;
	ymin = MIN(y1, y2);
	ymax = MAX(y1, y2) + 1;
	
	xmin = MAX(xmin, 0);
	ymin = MAX(ymin, 0);
	xmax = MIN(xmax, TERR_TEX_SZ);
	ymax = MIN(ymax, TERR_TEX_SZ);
	
	min = -999999;
	max = 999999;
	total = 0;
	
	count = (xmax - xmin + 1) * (ymax - ymin + 1);
	
	for(y = ymin; y <= ymax; y++) {
		for(x = xmin; x <= xmax; x++) {
			min = fmin(min, tb->zs[TCOORD(x,y)]);
			max = fmax(max, tb->zs[TCOORD(x,y)]);
			total += tb->zs[TCOORD(x,y)];
		}
	}
	
	ass->min = min;
	ass->max = max;
	ass->avg = total / count;
	
	xmax--;
	ymax--;
	// area in x-y plane only
	ass->areaFlat = (xmax - xmin) * (ymax - ymin);
	ass->volume = 0;
	
	for(y = ymin; y <= ymax; y++) {
		for(x = xmin; x <= xmax; x++) {
			float h1 = tb->zs[TCOORD(x,y)];
			float h2 = tb->zs[TCOORD(x+1,y)];
			float h3 = tb->zs[TCOORD(x,y+1)];
			float h4 = tb->zs[TCOORD(x+1,y+1)];
			
			/*
			split the volume in half along the diagonal of the base,
			leaving two weird prisms.
			
			find the lowest vertex of the top of each prism and cut it there,
			leaving a right triangular prism and an irregular pyramid on its side.
			the pyramid's base is irregular and created by the plane from step 1.
			*/
			
			float h23min = fmin(h2, h3);
			float p1min = fmin(h23min, h1);
			float p2min = fmin(h23min, h4);
			
			// each tile is a unit square, so the area of the base of each prism is .5
			float rtp1_v = p1min * .5;
			float rtp2_v = p2min * .5;
			
			// the base of the pyramid is a trapezoid, height sqrt(2)
			float p1ba = M_SQRT2 * (((h2 - p1min) + (h3 - p1min)) * .5);
			float p2ba = M_SQRT2 * (((h2 - p2min) + (h3 - p2min)) * .5);
			
			//               height     base area
			float p1_v = ((M_SQRT2 * .5 * p1ba) / 3.0);
			float p2_v = ((M_SQRT2 * .5 * p2ba) / 3.0);
			
			// add it all up
			ass->volume += p1_v + p2_v + rtp1_v + rtp2_v;
		}
	}
	
}


// ehhh... this was built before i realized you can't update parts of textures the way i thought you could.
void invalidateTerrain(TerrainBlock *tb, int x1, int y1, int x2, int y2) {
	int xmin, ymin, xmax, ymax;
	
	xmin = MIN(x1, x2);
	xmax = MAX(x1, x2);
	ymin = MIN(y1, y2);
	ymax = MAX(y1, y2);
	
	xmin = MAX(xmin, 0);
	ymin = MAX(ymin, 0);
	xmax = MIN(xmax, TERR_TEX_SZ - 1);
	ymax = MIN(ymax, TERR_TEX_SZ - 1);
	
	if(!tb->dirty) {
		tb->dirtyBox.min.x = xmin;
		tb->dirtyBox.min.y = ymin;
		tb->dirtyBox.max.x = xmax;
		tb->dirtyBox.max.y = ymax;
		
		tb->dirty = 1;
		return;
	}
	
	tb->dirtyBox.min.x = MIN(tb->dirtyBox.min.x, xmin);
	tb->dirtyBox.min.y = MIN(tb->dirtyBox.min.y, ymin);
	tb->dirtyBox.max.x = MAX(tb->dirtyBox.max.x, xmax);
	tb->dirtyBox.max.y = MAX(tb->dirtyBox.max.y, ymax);
}



void flattenArea(TerrainBlock *tb, int x1, int y1, int x2, int y2) {
	int x, y, xmin, ymin, xmax, ymax;
	AreaStats ass;
	
	xmin = MIN(x1, x2);
	xmax = MAX(x1, x2);
	ymin = MIN(y1, y2);
	ymax = MAX(y1, y2);
	
	xmin = MAX(xmin, 0);
	ymin = MAX(ymin, 0);
	xmax = MIN(xmax, TERR_TEX_SZ - 1);
	ymax = MIN(ymax, TERR_TEX_SZ - 1);
	
	areaStats(tb, x1,y1,x2,y2, &ass);
	
	printf("flattening: %d,%d|%d,%d to %f\n",xmin,ymin,xmax,ymax, ass.avg);
	// something seems off here, there might be a mismatch in resolution
	for(y = ymin; y <= ymax; y++) {
		for(x = xmin; x <= xmax; x++) {
			tb->zs[TCOORD(x,y)] = ass.avg;
		}
	}
	
	invalidateTerrain(tb, x1,y1,x2,y2);
	
	
}


void setZone(MapInfo *mi, int x1, int y1, int x2, int y2, int zone) {
	int x, y, xmin, ymin, xmax, ymax;
	
	xmin = MIN(x1, x2);
	xmax = MAX(x1, x2);
	ymin = MIN(y1, y2);
	ymax = MAX(y1, y2);
	
	xmin = MAX(xmin, 0);
	ymin = MAX(ymin, 0);
	xmax = MIN(xmax, MAP_TEX_SZ);
	ymax = MIN(ymax, MAP_TEX_SZ);
	
	printf("rezoning: %d,%d|%d,%d to %d\n",xmin,ymin,xmax,ymax, zone);
	
	/*
	for(y = ymin; y < ymax; y++) {
		for(x = xmin; x < xmax; x++) {
			mi->mb->zones[MCOORD(x,y)] = zone;
		}
	}
	
	
	
	
	mi->mb->dirtyZone = 1;
	*/
}



void getTerrainHeight(MapInfo* map, Vector2i* coords, int coordLen, float* heightsOut) {
	int i;
	
	
	for(i = 0; i < coordLen; i++) {
		
		
		
		
	}
	
	
	
};


// calculate a tile's center point in world coordinates
void tileCenterWorld(MapInfo* map, int tx, int ty, Vector* out) {
	
	tx = iclamp(tx, 0, TERR_TEX_SZ);
	ty = iclamp(ty, 0, TERR_TEX_SZ);
	
	float z = map->originMB->tb.zs[tx + (ty * TERR_TEX_SZ)];
	
	out->x = tx;
	out->y = ty;
	out->z = z; // .05
	
}


// sort y low to high, then x low to high
// terrible function name, can't think of a better one
void vSortLtoH42i(Vector2i* v) {
	
#define SWAP(v, p, a, b) \
	if(v[a].p > v[b].p) vSwap2i(&v[a], &v[b])
	
	// sort on y first
	// sorting networks are awesome
	SWAP(v, y, 0, 1);
	SWAP(v, y, 2, 3);
	SWAP(v, y, 0, 2);
	SWAP(v, y, 1, 3);
	SWAP(v, y, 1, 2);
	
	// this function never has to handle pathological degenerate cases of quads - points or lines
	// therefore more than two points will never be on the same axis
	if(v[0].y == v[1].y) SWAP(v, x, 0, 1);
	if(v[1].y == v[2].y) SWAP(v, x, 1, 2);
	if(v[2].y == v[3].y) SWAP(v, x, 2, 3);
	
#undef SWAP
}

/*

void slopeBetween(MapInfo* map, Vector2* p1, Vector2* p2, float width) {
	
	
	float* zs = map->tb->zs;
	
	// step 1: calculate exact corner coordinates
	Vector2 corners[4];
	Vector2 d, p;
	Quad2i ci;
	
	vSub2(p1, p2, &d);
	
	// get a perpendicular vector
	p.x = d.y;
	p.y = -d.x;
	
	vNorm2(&p, &p);
	vScale2(&p, width / 2, &p);
	
	//BUG: winding is probably wrong
	vAdd2(p1, &p, &corners[0]);
	vSub2(p1, &p, &corners[1]);
	vSub2(p2, &p, &corners[2]);
	vAdd2(p2, &p, &corners[3]);
	
	// step 2: get bounding corner tile coordinates
	quadRoundOutward2((Quad2*)corners, &ci);
	
	
	
	// check for degenerate cases
	
	// just a point, do nothing
	if(quadIsPoint2i(&ci)) {
		
		
		return;
	}
	
	
	// check for axis-aligned rectangles
	if(quadIsAARect2i(&ci)) {
		for() {
			for() {
			
			
			}
		}
		
		
		return;
	}
	
	// anything past here will be roughly diamond shaped
	
	
	// step 3: get start/stop heights
	float h1 = zs[MCOORD(p1->x,p1->y)];
	float h2 = zs[MCOORD(p2->x,p2->y)];
	
	// step 4: set new heights of affected tiles
	float rise = h1 - h2;
	float dx = rise / (p1.x - p2.x);
	float dy = rise / (p1.y - p2.y);
	
	
	
	// this is effectively a quad rasterizer
	
	int ytop, ybottom;
	int l_mid, r_mid;
	float lt_slope, lb_slope;
	float rt_slope, rb_slope;
	float l_slope, r_slope;
	
	int xleft, xright;
	
	
	ytop = .5 + fmax(fmax(corners[0].y, corners[1].y), fmax(corners[2].y, corners[3].y));
	ybottom = -0.5 + fmin(fmin(corners[0].y, corners[1].y), fmin(corners[2].y, corners[3].y));
	
	// midpoints are y coords of the left- and right-most vertices
	int i, j;
	for(i = j = 0; i < 4; i++)
		if(corners[i].x < corners[j].x) j = i;
	l_mid = corners[j].y;
	
	
	for(i = j = 0; i < 4; i++)
		if(corners[i].x > corners[j].x) j = i;
	r_mid = corners[j].y;
	
	// more magic from the diagram
	
	
	
	l_slope = lt_slope;
	r_slope = rt_slope;
	
	int x, y;
	for(y = ytop; y <= ybottom; y--) {
		
		// switch slopes at the midpoint
		if(y == r_mid) r_slope = rb_slope;
		if(y == l_mid) l_slope = lb_slope;
		
		for(x = xleft; x <= xright; x++) {
			
			// stuff
			
		}
		
	}
	
}
*/


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



