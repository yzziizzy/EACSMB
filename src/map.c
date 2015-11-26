 
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
	msScale3f(1024,1024,1024, &model);
	msPush(&model);
	
	mi->tb = allocTerrainBlock(0, 0);
	mi->mb = allocMapBlock(0, 0);
	
	
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
	
	
	glerr("clearing before terrain program load");
	terrProg = loadCombinedProgram("terrain");
	terrDepthProg = loadCombinedProgram("terrainDepth");
	
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
	zoneColors_ul = glGetUniformLocation(terrProg->id, "sZoneColors");
	glerr("terrain uniform loc ws");
	map_ul = glGetUniformLocation(terrProg->id, "sMap");
	glerr("terrain uniform loc ws");


	model_d_ul = glGetUniformLocation(terrDepthProg->id, "mModel");
	glerr("terraindepth uniform loc Model");
	view_d_ul = glGetUniformLocation(terrDepthProg->id, "mView");
	glerr("terraindepth uniform loc View");
	proj_d_ul = glGetUniformLocation(terrDepthProg->id, "mProj");
	glerr("terraindepth uniform loc Projection");
	heightmap_d_ul = glGetUniformLocation(terrDepthProg->id, "sHeightMap");
	glerr("terraindepth uniform loc hm");
	
	
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




MapBlock* allocMapBlock(int llx, int lly) {
	
	
	MapBlock* b = calloc(sizeof(MapBlock), 1);
	
	
	return b;
}



TerrainBlock* allocTerrainBlock(int cx, int cy) {
	
	
	TerrainBlock* tb;
	FILE* f;
	
	// clear for now
	tb = calloc(sizeof(TerrainBlock), 1);
	
	tb->cx = cx;
	tb->cy = cy;
	
	// BUG: probably wrong
	tb->box.min.x = cx - (TERR_BLOCK_SZ / 2);
	tb->box.max.x = cx + (TERR_BLOCK_SZ / 2);
	tb->box.min.y = cy - (TERR_BLOCK_SZ / 2);
	tb->box.max.y = cy + (TERR_BLOCK_SZ / 2);
	
	tb->tex = 0;
	
	f = fopen(tmpSavePath, "rb");
	
	if(f) {
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
				float f = PerlinNoise_2D(x / 512.0, y / 512.0, .1, 6); // slow-ass function, disable except for noise testing
	// 			printf("[%d,%d] %f\n", x,y,f);
				tb->zs[x + (y * TERR_TEX_SZ)] = fabs(1-f) * 150;
			}
		}
		
		printf("done.\nSaving terrain to %s\n", tmpSavePath);
		f = fopen(tmpSavePath, "wb");
		if(!f) return tb;
		
		fwrite(tb->zs, sizeof(float), TERR_TEX_SZ * TERR_TEX_SZ, f);
		
		fclose(f);
	}
	
	
	return tb;
}




void checkMapDirty(MapInfo* mi) {
	if(mi->mb->dirtyZone || mi->mb->dirtySurface) {
		updateMapTextures(mi->mb);
		mi->mb->dirtyZone = 0;
		mi->mb->dirtySurface = 0;
	}
	
	if(mi->tb->dirty) {
		updateTerrainTexture(mi->tb);
		mi->tb->dirty = 0;
	}
}


void updateMapTextures(MapBlock* mb) {
	if(!mb->tex) {
		
		glGenTextures(1, &mb->tex);
		glBindTexture(GL_TEXTURE_2D_ARRAY, mb->tex);
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
			2); // layers
		
		glexit("failed to create map textures");
	} 
	else {
		glBindTexture(GL_TEXTURE_2D_ARRAY, mb->tex);
	}
	
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
		0,  // mip level, 0 = base, no mipmap,
		0, 0, 0,// offset
		MAP_TEX_SZ,
		MAP_TEX_SZ,
		1,
		GL_RED_INTEGER,  // format
		GL_UNSIGNED_BYTE, // input type
		mb->zones);

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // target
		0,  // mip level, 0 = base, no mipmap,
		0, 0, 1,// offset
		MAP_TEX_SZ,
		MAP_TEX_SZ,
		1,
		GL_RED_INTEGER,  // format
		GL_UNSIGNED_BYTE, // input type
		mb->surface);
	
	glerr("failed to update map tex info");
}


void updateTerrainTexture(TerrainBlock* tb) {
	
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
	
	glGenTextures(1, &tb->tex);
	printf("tex num: %d \n", tb->tex);
	glBindTexture(GL_TEXTURE_2D, tb->tex);
	
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




void drawTerrainDepth(MapInfo* mi, Matrix* mView, Matrix* mProj) {
	
	drawTerrainBlockDepth(mi, msGetTop(&model), mView, mProj);
}



void drawTerrain(MapInfo* mi, Matrix* mView, Matrix* mProj, Vector2* cursor) {
	
	drawTerrainBlock(mi, msGetTop(&model), mView, mProj, cursor);
}


void drawTerrainBlock(MapInfo* mi, Matrix* mModel, Matrix* mView, Matrix* mProj, Vector2* cursor) {
	
	TerrainBlock* tb = mi->tb;
	
	glUseProgram(terrProg->id);
	glexit("using terrain program");
	
	glEnable(GL_DEPTH_TEST);
	
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

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_1D, mi->zoneColorTex);
	glUniform1i(zoneColors_ul, 2);
	glexit("bind zone colors texture");

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->mb->tex);
	glUniform1i(map_ul, 3);
	glexit("bind map info texture");


	glUniform1i(heightmap_ul, 0);
	glexit("hm sampler uniform");
	glUniform1i(basetex_ul, 1);
	glexit("base tex sampler uniform");

	
// 	l_ul = glGetUniformLocation(terrProg->id, "cursorPos");

	glUniform2f(glGetUniformLocation(terrProg->id, "cursorPos"), cursor->x, cursor->y);

	
	glerr("pre vao bind");
	glBindVertexArray(patchVAO);
	glerr("vao bind");
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
	glDrawArrays(GL_PATCHES, 0, totalPatches * totalPatches * 4);
	glerr("drawing");
	
	
}




void drawTerrainBlockDepth(MapInfo* mi, Matrix* mModel, Matrix* mView, Matrix* mProj) {
	
	TerrainBlock* tb = mi->tb;
	
	glUseProgram(terrDepthProg->id);
	glexit("using terrain program");
	
	glEnable(GL_DEPTH_TEST);
	
	glUniformMatrix4fv(model_d_ul, 1, GL_FALSE, mModel->m);
	glUniformMatrix4fv(view_d_ul, 1, GL_FALSE, mView->m);
	glUniformMatrix4fv(proj_d_ul, 1, GL_FALSE, mProj->m);
	glexit("terrain matrix uniforms");
	
	
	
	glUniform2f(winsize_ul, 600, 600);
	
	
	
	glActiveTexture(GL_TEXTURE0);
	
	glexit("active texture");
	
	glBindTexture(GL_TEXTURE_2D, tb->tex);
	glexit("bind hm texture");
	
// 	glActiveTexture(GL_TEXTURE1);
// 	glBindTexture(GL_TEXTURE_2D, cnoise->tex_id);
// 	glexit("bind base texture");
// 
// 	glActiveTexture(GL_TEXTURE2);
// 	glBindTexture(GL_TEXTURE_1D, mi->zoneColorTex);
// 	glUniform1i(zoneColors_ul, 2);
// 	glexit("bind zone colors texture");
// 
// 	glActiveTexture(GL_TEXTURE3);
// 	glBindTexture(GL_TEXTURE_2D_ARRAY, mi->mb->tex);
// 	glUniform1i(map_ul, 3);
// 	glexit("bind map info texture");


	glUniform1i(heightmap_d_ul, 0);
	glexit("hm sampler uniform");
// 	glUniform1i(basetex_ul, 1);
// 	glexit("base tex sampler uniform");

	
// 	l_ul = glGetUniformLocation(terrProg->id, "cursorPos");

// 	glUniform2f(glGetUniformLocation(terrProg->id, "cursorPos"), cursor->x, cursor->y);

	
	glerr("pre vao bind");
	glBindVertexArray(patchVAO);
	glerr("vao bind");
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindBuffer(GL_ARRAY_BUFFER, patchVBO);
	glDrawArrays(GL_PATCHES, 0, totalPatches * totalPatches * 4);
	glerr("drawing");
	
	
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
	
	for(y = ymin; y < ymax; y++) {
		for(x = xmin; x < xmax; x++) {
			mi->mb->zones[MCOORD(x,y)] = zone;
		}
	}
	
	
	
	
	mi->mb->dirtyZone = 1;
}



void getTerrainHeight(MapInfo* map, Vector2i* coords, int coordLen, float* heightsOut) {
	int i;
	
	
	for(i = 0; i < coordLen; i++) {
		
		
		
		
	}
	
	
	
};


// calculate a tile's center point in world coordinates
int tileCenterWorld(MapInfo* map, int tx, int ty, Vector* out) {
	
	tx = MAX(MIN(tx, TERR_TEX_SZ), 0);
	ty = MAX(MIN(ty, TERR_TEX_SZ), 0);
	
	float z = map->tb->zs[tx + (ty * TERR_TEX_SZ)];
	
	out->x = tx;
	out->y = ty;
	out->z = z; // .05
	
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



