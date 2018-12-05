 
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
#include "c_json/json.h"
#include "shader.h"
#include "texture.h"
#include "road.h"
#include "map.h"
#include "perlin.h"
#include "opensimplex.h"
//#include "terrain.h"




static GLuint patchVAO;
static GLuint patchVBO;
static GLuint proj_ul, view_ul, model_ul, heightmap_ul, offset_ul, winsize_ul, basetex_ul, diffuse_ul;
static GLuint proj_d_ul, view_d_ul, model_d_ul, heightmap_d_ul, offset_d_ul, winsize_d_ul;
static GLuint map_ul, zoneColors_ul;
static int totalPatches;
// Texture* cnoise;

// temp
//TerrainTexInfo tti;

static TerrainPatchVertex* patchVertices;

ShaderProgram* terrProg;
ShaderProgram* terrDepthProg;

GLuint MaxPatchVertices;
GLuint MaxTessGenLevel;

static MatrixStack model;

char* tmpSavePath;
char* tmpSaveName = "EACSMB-map-cache";


void initMap(MapInfo* mi) {
	int x, y, i;
	int bx, by;
	char* tmpDir;
	
	// HACK DEBUG. leaks like wiki
	
	//terrain_initTexInfo(&tti);
	//terrain_readConfigJSON(&tti, "assets/config/terrain.json");
	//terrain_loadTextures(&tti);
	
	tmpDir = "data"; // getenv("TMPDIR");
	
	if(!tmpDir) tmpDir = P_tmpdir;
	
	tmpSavePath = malloc(strlen(tmpDir) + strlen(tmpSaveName) + 2);
	
	strcpy(tmpSavePath, tmpDir);
	strcat(tmpSavePath, "/");
	strcat(tmpSavePath, tmpSaveName);
	
	
	
	initTerrain(mi);
	



	FILE* f;
	f = fopen(tmpSavePath, "rb");
	MapBlock* mb;
	if(f) {
		// load data
		printf("Loading saved terrain from %s\n", tmpSavePath);
		for(y = 0; y < 8; y++) {
			for(x = 0; x < 8; x++) {
				//mb = loadMapBlock(f);
				//mi->blocks[mb->bix][mb->biy] = mb;
				printf("fix me %s:%d\n", __FILE__, __LINE__);
			}
		}
	} 
	else {
		f = fopen(tmpSavePath, "wb");
		for(y = 0; y < 8; y++) {
			for(x = 0; x < 8; x++) {
				//mi->blocks[x][y] = spawnMapBlock(mi, x, y);
				//saveMapBlock(f, mi->blocks[x][y]);
				printf("fix me %s:%d\n", __FILE__, __LINE__);
			}
		}
	}
	
	fclose(f);
	
	updateTerrainTexture(mi);
	
	// HACK: this should be moved to a per-frame function later
	
	i = 0;
	for(y = 0; y < 8; y++) {
		for(x = 0; x < 8; x++) {
			mi->offsetData[i].x = x;
			mi->offsetData[i].y = y;
			printf("fix me %s:%d\n", __FILE__, __LINE__);
			i++;
		}
	}
	
 	glBindTexture(GL_TEXTURE_2D, mi->locationTex);
	glexit("");
	glTexSubImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		0, 0, // offset
		64,
		1,
		GL_RG,  // format
		GL_UNSIGNED_BYTE, // input type
		mi->offsetData);
	glexit("");
	
	mi->numBlocksToRender = 64;
}


void initTerrain(MapInfo* mi) {
	
	
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
	
	
//	cnoise = loadBitmapTexture("./assets/textures/grass_texture-256.png");
	
	// shader setup
	//-------------
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	
	
	// testing to see if gl vertex state if corrupting the shader
	static VAOConfig vao_opts[] = {
		// per vertex
		{0, 2, GL_FLOAT, 0, GL_FALSE}, // position
		{0, 2, GL_FLOAT, 0, GL_FALSE}, // tex
		
		// per instance 
		{1, 2, GL_FLOAT, 1, GL_FALSE}, // block offset
		
		{0, 0, 0}
	};
	
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
	
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	
	GLuint tempvao = makeVAO(vao_opts);
	glBindVertexArray(tempvao);
	
	
	
	
	glexit("before terrain progs");
	terrProg = loadCombinedProgram("terrain");
	printf("terrProg: %d\n", terrProg->id);
	glexit("mid terrain progs");
	terrDepthProg = loadCombinedProgram("terrainDepth");
	glexit("after terrain progs");
	
	model_ul = glGetUniformLocation(terrProg->id, "mModel");
	view_ul = glGetUniformLocation(terrProg->id, "mView");
	proj_ul = glGetUniformLocation(terrProg->id, "mProj");

	offset_ul = glGetUniformLocation(terrProg->id, "sOffsetLookup");
	heightmap_ul = glGetUniformLocation(terrProg->id, "sHeightMap");
	basetex_ul = glGetUniformLocation(terrProg->id, "sBaseTex");
	diffuse_ul = glGetUniformLocation(terrProg->id, "sDiffuse");
	winsize_ul = glGetUniformLocation(terrProg->id, "winSize");
	zoneColors_ul = glGetUniformLocation(terrProg->id, "sZoneColors");
	map_ul = glGetUniformLocation(terrProg->id, "sMap");
	glexit("");


	model_d_ul = glGetUniformLocation(terrDepthProg->id, "mModel");
	view_d_ul = glGetUniformLocation(terrDepthProg->id, "mView");
	proj_d_ul = glGetUniformLocation(terrDepthProg->id, "mProj");
	offset_d_ul = glGetUniformLocation(terrDepthProg->id, "sOffsetLookup");
	heightmap_d_ul = glGetUniformLocation(terrDepthProg->id, "sHeightMap");
	winsize_d_ul = glGetUniformLocation(terrDepthProg->id, "winSize");
	glexit("");
	
	// texture units don't change
	glProgramUniform1i(terrProg->id, heightmap_ul, 21);
	glProgramUniform1i(terrProg->id, basetex_ul, 22);
	glProgramUniform1i(terrProg->id, diffuse_ul, 23);
	glProgramUniform1i(terrProg->id, offset_ul, 20);

	glProgramUniform1i(terrDepthProg->id, heightmap_d_ul, 21);
	glProgramUniform1i(terrDepthProg->id, offset_d_ul, 20);
	

	glexit("before terrain patch");

	
	
	glGenTextures(1, &mi->locationTex);
	glBindTexture(GL_TEXTURE_2D, mi->locationTex);
	glexit("failed to create offset textures b");
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glexit("failed to create offset textures a");
	
	// squash the data in
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexStorage2D(GL_TEXTURE_2D,
		1,  // mips, flat
		GL_RG8,
		64, 1
		); 
	
	glexit("failed to create offset textures");
	
	mi->offsetData = calloc(1, sizeof(struct sGL_RG8) * 64);
	mi->numBlocksToRender = 0;
	
	
}





MapBlock* allocMapBlock(int bix, int biy) {
	
	
	MapBlock* b = calloc(sizeof(MapBlock), 1);
	b->w = bix;
	b->h = biy;
	
	return b;
}



void initTerrainBlock(MapBlock* mb, int cx, int cy) {
	
	printf("initTerrainBlock is deprecated\n");
	int x, y;
	
	TerrainBlock* tb = 0;//&mb->tb;
	printf("fix me %s:%d\n", __FILE__, __LINE__);
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
	
	OpenSimplexNoise osn;
	OpenSimplex_init(&osn, 6456, 512, 512);
	
	OpenSimplexOctave octs[] = {
		{2, 1.0},
		{4, 0.7},
		{8, 0.4},
		{16, 0.2},
		{32, 0.05},
		{-1, -1}
	};
	OpenSimplexParams params = {
		TERR_TEX_SZ, TERR_TEX_SZ,
		offx + 1024*512, offy + 1024*512,
		octs
	};
	
	float* data = OpenSimplex_GenNoise2D(&osn, &params);
	
	
	for(y = 0; y < TERR_TEX_SZ ; y++) {
		for(x = 0; x < TERR_TEX_SZ ; x++) {
			float f = data[x + (y * TERR_TEX_SZ)];
			tb->zs[x + (y * TERR_TEX_SZ)] = fabs(1-f) * 10;
		}
	}
	
	free(data);
	/*
		// generate new data and save it
	printf("Generating new terrain [%d, %d]... \n", cx, cy);
	int x, y;
	for(y = 0; y < TERR_TEX_SZ ; y++) {
		for(x = 0; x < TERR_TEX_SZ ; x++) {
			//tb->zs[x + (y * TERR_TEX_SZ)] = sin(x * .1) * .1;
			float f = PerlinNoise_2D((offx + x) / 128.0, (offy + y) / 128.0, .1, 6); // slow-ass function, disable except for noise testing
// 			printf("[%d,%d] %f\n", x,y,f);
			tb->zs[x + (y * TERR_TEX_SZ)] = fabs(1-f) * 100;
		}
	}
	*/
	
}






void updateMapTextures(MapInfo* mi) {
	if(!mi->tex) {
		
		glGenTextures(1, &mi->tex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, mi->tex);
		glexit("failed to create map textures b");
		
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
			mi->block->w, mi->block->h,
			2); // layers
		
		glexit("failed to create map textures");
	}
	else {
		glBindTexture(GL_TEXTURE_2D_ARRAY, mi->tex);
	}

	if(!mi->zones) mi->zones = malloc(mi->block->w * mi->block->h);
	for(int y = 0; y < mi->block->h; y++) {
		for(int x = 0; x < mi->block->w; x++) {
			mi->zones[x + (y * mi->block->w)] = (y+x) % 3;
			//printf("%d", mi->zones[x + (y * mi->block->w)]);
		}
		//printf("\n");
	}
	
	
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
		0,  // mip level, 0 = base, no mipmap,
		0, 0, 0,// offset
		mi->block->w,
		mi->block->h,
		1,
		GL_RED_INTEGER,  // format
		GL_UNSIGNED_BYTE, // input type
		mi->zones);
	
	printf("sz: %d, %d\n", mi->block->w, mi->block->h);
/*	
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
			/*mi->texIndexMap[i]->surface 9999);
	}*/
	printf("fix me ^ %s:%d\n", __FILE__, __LINE__);
	
	glerr("failed to update map tex info");
}




void Map_updateSurfaceTextures(MapInfo* mi) {
	
	MapLayer* ml_surface = MapBlock_GetLayer(mi->block, "surface");
	if(!ml_surface) return; 
	
	if(glGenBindTexture(&mi->tex, GL_TEXTURE_2D_ARRAY)) {
		
		texParams2D(GL_TEXTURE_2D_ARRAY, GL_NEAREST, GL_CLAMP_TO_EDGE);
		
		// squash the data in
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		glTexStorage3D(GL_TEXTURE_2D_ARRAY,
			1,  // mips, flat
			GL_R8UI,
			ml_surface->w, ml_surface->h,
			2); // layers
		
		glexit("failed to create map textures");
	}
	
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
		0,  // mip level, 0 = base, no mipmap,
		0, 0, 0,// offset
		ml_surface->w, ml_surface->h, 1,
		GL_RED_INTEGER,  // format
		GL_UNSIGNED_BYTE, // input type
		ml_surface->data.uc);
	
	glexit("");
}



void updateTerrainTexture(MapInfo* mi) {
	int x, y, i;
	MapLayer* ml = mi->block->terrain;
	
	
	if(glGenBindTexture(&mi->terrainTex, GL_TEXTURE_2D_ARRAY)) {
		
		texParams2D(GL_TEXTURE_2D_ARRAY, GL_NEAREST, GL_CLAMP_TO_EDGE);
		
		// squash the data in
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		
		glTexStorage3D(GL_TEXTURE_2D_ARRAY,
			1,  // mips, flat
			GL_R32F,
			ml->w, ml->h,
			4); // layers: terrain, water 1&2, soil
		
		glexit("failed to create map textures");
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
	
printf("loading terrain data\n");
//printf("fix me %s:%d\n", __FILE__, __LINE__);
	
// 	if(!ml->data.f) ml->data.f = calloc(1, sizeof(float) * ml->w * ml->h);
// 
// 	for(int y = 0; y < ml->h; y++)
// 	for(int x = 0; x < ml->w; x++) {
// 		ml->data.f[x + (y * ml->w)] = 0;
// 	}



	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
		0,  // mip level, 0 = base, no mipmap,
		0, 0, 0,// offset
		ml->w,
		ml->h,
		1,
		GL_RED,  // format
		GL_FLOAT, // input type
		ml->data.f);

glexit("");

	
	MapLayer* ml_water = MapBlock_GetLayer(mi->block, "water");
	if(ml_water) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
			0, 0, 1,// offset
			ml->w, ml->h, 1,
			GL_RED,	GL_FLOAT, ml_water->data.f);
	}
	else {
		printf("!!! water tex not found\n");
	}
		
	MapLayer* ml_water2 = MapBlock_GetLayer(mi->block, "water2");
	if(ml_water2) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
			0, 0, 2,// offset
			ml->w, ml->h, 1,
			GL_RED,	GL_FLOAT, ml_water2->data.f);
	}
	else {
		printf("!!! water2 tex not found\n");
	}
	
	MapLayer* ml_soil = MapBlock_GetLayer(mi->block, "soil");
	if(ml_soil) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
			0, 0, 3,// offset
			ml->w, ml->h, 1,
			GL_RED,	GL_FLOAT, ml_soil->data.f);
	}
	else {
		printf("!!! soil tex not found\n");
	}
	
		
	glerr("updating terrain tex info");
}


static void bindTerrainTextures(MapInfo* mi, GLuint progID) {
	
	glActiveTexture(GL_TEXTURE0 + 19);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->tex);
	glProgramUniform1i(progID, glGetUniformLocation(progID, "sData"), 19);

	glActiveTexture(GL_TEXTURE0 + 21);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->terrainTex);
	GLuint hmul = glGetUniformLocation(progID, "sHeightMap");
	glProgramUniform1i(progID, hmul, 21);
	
	glActiveTexture(GL_TEXTURE0 + 22);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->tm->tex_id);
	GLuint texul = glGetUniformLocation(progID, "sTextures");
	glProgramUniform1i(progID, texul, 22);
	
	
	//MapBlock_GetLayer(mi->block, "surface");
	
	
	//glActiveTexture(GL_TEXTURE0 + 22);
	//glBindTexture(GL_TEXTURE_2D, cnoise->tex_id);

	//glActiveTexture(GL_TEXTURE0 + 23);
//	glBindTexture(GL_TEXTURE_2D_ARRAY, tti.diffuse.tex_id);
	
	glActiveTexture(GL_TEXTURE0 + 20);
	glBindTexture(GL_TEXTURE_2D, mi->locationTex);
	glexit("");
}






void drawTerrainDepth(MapInfo* mi, UniformBuffer* perViewUB, Vector2* viewWH) {
	int i;
	
	glUseProgram(terrDepthProg->id);
	//glDisable(GL_DEPTH_TEST);

	uniformBuffer_bindProg(perViewUB, terrDepthProg->id, "perViewData");
	
// 	glUniformMatrix4fv(view_d_ul, 1, GL_FALSE, mView->m);
// 	glexit("");
// 	glUniformMatrix4fv(proj_d_ul, 1, GL_FALSE, mProj->m);
// 	glexit("");
	glUniform2f(winsize_d_ul, viewWH->x, viewWH->y);
	glexit("");
	
	bindTerrainTextures(mi, terrDepthProg->id);
	
glexit("");	
	glBindVertexArray(patchVAO);
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
	
	glUniformMatrix4fv(model_d_ul, 1, GL_FALSE, msGetTop(&model)->m);
glexit("");
	glDrawArraysInstanced(GL_PATCHES, 0, totalPatches * totalPatches * 4, mi->numBlocksToRender);
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
	
	//printf("flattening: %d,%d|%d,%d to %f\n",xmin,ymin,xmax,ymax, ass.avg);
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
	printf("!!! broken dead code: setZone()\n");
}

// calculate a tile's center point in world coordinates
void tileCenterWorld(MapInfo* map, int tx, int ty, Vector* out) {
	//printf("!!! broken dead code: tileCenterWorld() \n");
	
	tx = iclamp(tx, 0, TERR_TEX_SZ);
	ty = iclamp(ty, 0, TERR_TEX_SZ);
	
	
	printf("fix me %s:%d\n", __FILE__, __LINE__);
	float z = 0;//map->blocks[0][0]->tb.zs[tx + (ty * TERR_TEX_SZ)];
	
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










void Map_readConfigFile(MapInfo* mi, char* path) {
	
	json_file_t* jsf;
	jsf = json_load_path(path);
	
	size_t l, i;
	
	int ret;
	struct json_value* tex_o;
	struct json_obj* o;
	void* iter;
	char* key, *texName, *tmp;
	struct json_value* v, *tc;

	
	// TODO: error handling
	if(jsf->root->type != JSON_TYPE_OBJ) {
		printf("invalid terrain tex config format\n");
		return;
	}

	
	json_obj_get_key(jsf->root, "types", &tex_o);
	if(tex_o->type != JSON_TYPE_OBJ) {
		printf("invalid terrain tex config format (2)\n");
		return;
	}
	
	
	iter = NULL;
	while(json_obj_next(tex_o, &iter, &key, &tc)) {
		char* path;
		MapSurfaceType* st = pcalloc(st); 
		
		st->name = strdup(key);
		st->diffuse = -1;
		st->normal = -1;
		VEC_PUSH(&mi->surfaceTypes, st);
		
		if(tc->type == JSON_TYPE_OBJ) {
			json_obj_get_key(tc, "diffuse", &v);
			if(v->type == JSON_TYPE_STRING) {
				json_as_string(v, &path);
				st->diffuse = TextureManager_reservePath(mi->tm, path);
			}
			else {
				fprintf(stderr, "!!! Terrain type '%s' missing diffuse texture.\n", key);
			}
			
			json_obj_get_key(tc, "normal", &v);
			if(v->type == JSON_TYPE_STRING) {
				json_as_string(v, &path);
				st->normal = TextureManager_reservePath(mi->tm, path);
			}
		}
		else {
			
			json_as_string(tc, &path);
			
			st->diffuse = TextureManager_reservePath(mi->tm, path);
		}
		
		
	}
	
	// fill in uniform block
	VEC_EACH(&mi->surfaceTypes, i, st) {
		mi->surfaceUniforms[i].diffuse = st->diffuse; 
		mi->surfaceUniforms[i].normal = st->normal; 
	}
	
	
}




static void uniformSetup(MapInfo* mi, GLuint progID) {
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	//	glUniform3f(glGetUniformLocation(terrProg->id, "cursorPos"), m->cursorPos.x, m->cursorPos.y, m->cursorPos.z);
//	glUniform2f(winsize_ul, pdp->targetSize.x, pdp->targetSize.y);

	//glUniformMatrix4fv(model_ul, 1, GL_FALSE, msGetTop(&model)->m);
	
	static int waterIndex = 0;
	waterIndex = (waterIndex + 1) % 2;
	glUniform1i(glGetUniformLocation(progID, "waterIndex"), waterIndex);
	
	// TODO: move elsewhere, no need to set every frame
	glUniform2iv(glGetUniformLocation(progID, "aSurfaces"), 16, mi->surfaceUniforms);
	
	bindTerrainTextures(mi, progID);
}


static void instanceSetup(MapInfo* mi, TerrainPatchInstance* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp) {
	
	int i, j;
	int x, y;
	
	for(j = 0; j < diCount; j++) {
		di[j]->numToDraw = 4;
		
		i = 0;
		for(y = 0; y < 2; y++) { // each instance
			for(x = 0; x < 2; x++) { // each instance
				vmem[i].offx = x; // multiplier should be mb->w
				vmem[i].offy = y;
				i++;
			}
		}
		
		di++;
	}
	
}







RenderPass* Map_CreateRenderPass(MapInfo* m) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = Map_CreateDrawable(m);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* Map_CreateDrawable(MapInfo* m) {
	return MultiDrawIndirect_CreateDrawable(m->blockPatch, terrProg);
}





RenderPass* Map_CreateSelectionPass(MapInfo* m) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = MultiDrawIndirect_CreateDrawable(m->blockPatch, terrDepthProg);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


RenderPass* Map_CreateShadowPass(MapInfo* m) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = MultiDrawIndirect_CreateDrawable(m->blockPatch, loadCombinedProgram("terrainShadow"));

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}





float Map_getTerrainHeight(MapInfo* mi, Vector2i p) {
	MapLayer* ml = mi->block->terrain;
	
	if(p.x >= ml->w || p.x < 0 || p.y >= ml->h || p.y < 0) return 0.0f;
	
	return ml->data.f[p.x + (p.y * ml->w)];
}



float Map_getTerrainHeight3f(MapInfo* mi, Vector p) {
	Vector2 p2 = {.x = p.x, .y = p.y};
	return Map_getTerrainHeightf(mi, p2);
}

float Map_getTerrainHeightf(MapInfo* mi, Vector2 p) {
	MapLayer* ml = mi->block->terrain;
	
	if(!isfinite(p.x) || !isfinite(p.y) || p.x >= ml->w || p.x < 0 || p.y >= ml->h || p.y < 0) return 0.0f;
	
	float cc = ml->data.f[(int)(ceil(p.x) + (ceil(p.y)) * ml->w)];
	float cf = ml->data.f[(int)(ceil(p.x) + (floor(p.y)) * ml->w)];
	float fc = ml->data.f[(int)(floor(p.x) + (ceil(p.y)) * ml->w)];
	float ff = ml->data.f[(int)(floor(p.x) + (floor(p.y)) * ml->w)];
	
	float c = flerp(cc, cf, p.y - floor(p.y));
	float f = flerp(ff, fc, p.y - floor(p.y));
	return flerp(c, f, p.x - floor(p.x));
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





void MapLayer_init(MapLayer* ml, Vector2i size, float scale) {
	//ml->name = strdup(name);
	
	ml->w = size.x;
	ml->h = size.y;
	ml->scale = scale;
	//ml->chunkEdgeLen = 512;
	
	//ml->data_f = calloc(1, sizeof(*ml->data_f) * ml->w * ml->h);
}


MapLayer* MapLayer_Alloc(Vector2i size, float scale) {
	MapLayer* ml;
	
	pcalloc(ml);
	MapLayer_init(ml, size, scale);
	
	return ml;
}



float MapLayer_sample(MapLayer* ml, float x, float y) {
	
}







MapBlock* MapBlock_Alloc(int w, int h) {
	MapBlock* mb;
	
	pcalloc(mb);
	
	mb->w = w;
	mb->h = h;
	
	VEC_INIT(&mb->layers);
	HT_init(&mb->layerLookup, 4);
	
	return mb;
}


int MapBlock_AddLayer(MapBlock* mb, char* name, int scale, char type) {
	MapLayer* ml;
	int lw, lh;
	
	lw = mb->w / scale;
	lh = mb->h / scale;
	
	ml = MapLayer_Alloc((Vector2i){lw, lh}, 1.0 / (float)scale);
	ml->name = strdup(name);
	ml->dataType = type;
	
	VEC_PUSH(&mb->layers, ml);
	int i = VEC_LEN(&mb->layers) - 1;
	HT_set(&mb->layerLookup, name, i);
	
	if(strcaseeq(name, "terrain")) {
		mb->terrain = ml;
	}
	
	return i;
}


MapLayer* MapBlock_GetLayer(MapBlock* mb, char* name) {
	MapLayer* ml;
	int64_t i;
		
	if(HT_get(&mb->layerLookup, name, &i)) {
		return NULL;
	}
	
	return VEC_ITEM(&mb->layers, i);
}






void MapLayer_GenTerrain(MapLayer* ml, MapLayer* surface) {
	int x, y;
	FILE* f;
	
	Mapgen_v1(ml, surface);
}



void MapLayer_Fill(MapLayer* ml, float value) {
	int x, y;
	
	if(!ml->data.f)
		ml->data.f = malloc(sizeof(*ml->data.f) * ml->w * ml->h);
	
	for(y = 0; y < ml->h ; y++) {
		for(x = 0; x < ml->w ; x++) {
			// HACK: randomized for erosion tests
			ml->data.f[x + (y * ml->w)] = frand(value/2.0, value);
		}
	}
}


void MapLayer_FillUChar(MapLayer* ml, unsigned char value) {
	int x, y;
	
	if(!ml->data.uc)
		ml->data.uc = malloc(sizeof(*ml->data.uc) * ml->w * ml->h);
	
	for(y = 0; y < ml->h ; y++) {
		for(x = 0; x < ml->w ; x++) {
			ml->data.uc[x + (y * ml->w)] = value;
		}
	}
}




void MapInfo_Init(MapInfo* mi, GlobalSettings* gs) {
	static VAOConfig vao_opts[] = {
		// per vertex
		{0, 2, GL_FLOAT, 0, GL_FALSE}, // position
		{0, 2, GL_FLOAT, 0, GL_FALSE}, // tex
		
		// per instance 
		{1, 2, GL_FLOAT, 1, GL_FALSE}, // block offset
		
		{0, 0, 0}
	};
	
	mi->blockPatch = MultiDrawIndirect_alloc(vao_opts, 64);
	
	
	
	//mi->block = MapBlock_Alloc(1024, 1024);
	mi->block = MapBlock_Alloc(512, 512);
	//mi->block = MapBlock_Alloc(4096, 4096);
	
	MapBlock_AddLayer(mi->block, "terrain", 1, 0);
	MapBlock_AddLayer(mi->block, "water", 1, 0);
	MapBlock_AddLayer(mi->block, "water2", 1, 0);
	MapBlock_AddLayer(mi->block, "soil", 1, 0);
	
	// surface textures
	int si = MapBlock_AddLayer(mi->block, "surface", 1, 1);
	MapLayer* surface = VEC_ITEM(&mi->block->layers, si);
	MapLayer_FillUChar(surface, 1);
	
	
	MapLayer_Fill(MapBlock_GetLayer(mi->block, "water"), 8.0);
	MapLayer_Fill(MapBlock_GetLayer(mi->block, "water2"), 8.0);
	MapLayer_Fill(MapBlock_GetLayer(mi->block, "soil"), 0.0);
	
	MapLayer_GenTerrain(mi->block->terrain, surface);
}


void MapInfo_InitGL(MapInfo* mi, GlobalSettings* gs) {
	
	

	MultiDrawIndirect_initGL(mi->blockPatch);
	

	
	// temp, initializes the patches
	initTerrain(mi);
	
	MapInfo_GenMesh(mi);
	
	// does nothing atm
	MapInfo_initLayerTextures(mi);
	
	// gen heightmap
// 	MapLayer_GenTerrain(mi->block->terrain, surface);
	MapGen_initWaterVelTex(mi); 
	
	Map_updateSurfaceTextures(mi);
	updateTerrainTexture(mi);
	
// 	updateMapTextures(mi);
	
	int	x,y,i = 0;
	for(y = 0; y < 8; y++) {
		for(x = 0; x < 8; x++) {
			mi->offsetData[i].x = x;
			mi->offsetData[i].y = y;
			//printf("fix me %s:%d\n", __FILE__, __LINE__);
			i++;
		}
	}
	
 	glBindTexture(GL_TEXTURE_2D, mi->locationTex);
	glexit("");
	glTexSubImage2D(GL_TEXTURE_2D, // target
		0,  // level, 0 = base, no minimap,
		0, 0, // offset
		64,
		1,
		GL_RG,  // format
		GL_UNSIGNED_BYTE, // input type
		mi->offsetData);
	glexit("");
	
	mi->numBlocksToRender = 64;
	
	
}

void MapInfo_initLayerTextures(MapInfo*mi) {
	
	
	
	
}


void MapInfo_GenMesh(MapInfo* mi) {
	
 	GLint MaxTessGenLevel = 0;
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
	
	//MapBlock* mb = mi->block;
	
	
	
	Vector2i totalPatches = {
		ceil(256.0 / MaxTessGenLevel), //wholePatches + (fracPatchSize > 0 ? 1 : 0);
		ceil(256.0 / MaxTessGenLevel)
	};
	
	int patchCnt = (totalPatches.x * totalPatches.y);
	TerrainPatchVertex* patchVertices = calloc(1, sizeof(TerrainPatchVertex) * 4 * patchCnt);
	
	
	float sideUnit = 1.0; // size of a square, the smallest tessellation level
	float patchSide = sideUnit * MaxTessGenLevel;

	
	
	TerrainPatchVertex* pv = patchVertices;
	
	int ix, iy;
	for(iy = 0; iy < totalPatches.y; iy++) {
		for(ix = 0; ix < totalPatches.x; ix++) {
			
			pv->x = ((ix + 0) * patchSide);
			pv->y = ((iy + 0) * patchSide);
			pv->u = pv->x / 256.0;//mb->w;
			pv->v = pv->y / 256.0;//mb->h;
			//printf("x:%f  y:%f  u:%f  v:%f \n", pv->x, pv->y, pv->u, pv->v);
			pv++;
			
			pv->x = ((ix + 0) * patchSide);
			pv->y = ((iy + 1) * patchSide);
			pv->u = pv->x / 256.0;//mb->w;
			pv->v = pv->y / 256.0;//mb->h;
			//printf("x:%f  y:%f  u:%f  v:%f \n", pv->x, pv->y, pv->u, pv->v);
			pv++;	
			
			pv->x = ((ix + 1) * patchSide);
			pv->y = ((iy + 1) * patchSide);
			pv->u = pv->x / 256.0;//mb->w;
			pv->v = pv->y / 256.0;//mb->h;
			//printf("x:%f  y:%f  u:%f  v:%f \n", pv->x, pv->y, pv->u, pv->v);
			pv++;	
			
			pv->x = ((ix + 1) * patchSide);
			pv->y = ((iy + 0) * patchSide);
			pv->u = pv->x / 256.0;//mb->w;
			pv->v = pv->y / 256.0;//mb->h;
			//printf("x:%f  y:%f  u:%f  v:%f \n", pv->x, pv->y, pv->u, pv->v);
			pv++;	
			
			//printf("\n");
		}
	}
	
	
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	MDIDrawInfo* di = pcalloc(di);
	
	*di = (MDIDrawInfo){
		.vertices = patchVertices,
		.vertexCount = patchCnt * 4,
		
		.indices = NULL,
		.indexCount = 0,
	};
	
	mi->blockPatch->isIndexed = 0;
	mi->blockPatch->primMode = GL_PATCHES;
	mi->blockPatch->uniformSetup = (void*)uniformSetup;
	mi->blockPatch->instanceSetup = (void*)instanceSetup;
	mi->blockPatch->data = mi;
	
	MultiDrawIndirect_addMesh(mi->blockPatch, di);
	MultiDrawIndirect_updateGeometry(mi->blockPatch);
}


