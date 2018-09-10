#ifndef __EACSMB_MAP_H__
#define __EACSMB_MAP_H__


#define MAP_SIZE (255 * 8)

// terrain textures must always be a power of two
#define TERR_TEX_SZ 256
// terrain blocks must always be a power of two minus one
#define TERR_BLOCK_SZ (TERR_TEX_SZ - 1)

#define MAP_TEX_SZ (TERR_TEX_SZ)
#define MAP_BLOCK_SZ (TERR_TEX_SZ - 1)




#define TCOORD(x,y) ((y * TERR_TEX_SZ) + x)
#define MCOORD(x,y) ((y * MAP_TEX_SZ) + x)

#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "pass.h"
#include "texture.h"
#include "mdi.h"

#include "road.h"
#include "view.h"
#include "uniformBuffer.h"


struct MapInfo;
typedef struct MapInfo MapInfo;











struct MapBlockTreeLeaf;
typedef struct MapBlockTreeLeaf MapBlockTreeLeaf;


typedef struct TerrainPatchVertex {
	float x, y;
	float u, v;
} TerrainPatchVertex;

typedef struct TerrainPatchInstance {
	float offx, offy;
} TerrainPatchInstance;


typedef struct TerrainBlock {
	float zs[TERR_TEX_SZ * TERR_TEX_SZ];
	int cx, cy;
	AABB2 box;
	int scale;
	
	int dirty;
	AABB2 dirtyBox;
	
	GLuint tex;
	
} TerrainBlock;







typedef struct MapLayer {
	char* name;
	
	int w, h; // tex size
	float scale; // baked ratio 
	
	char dataType;
	union {
		char* c;
		unsigned char* uc;
		int* i;
		unsigned int* ui;
		float* f;
	} data;
	
} MapLayer;





typedef struct MapBlock {
	//int32_t bix, biy; // location in blocks. max world size is 2^32 blocks square ~5.6e8 km2 ~1.1x earth surface area  
	
	int w, h;
	
	VEC(MapLayer*) layers;
	HashTable(int) layerLookup;
	
	MapLayer* terrain; // shortcut for heightmap
	
	
	// temp hack
	GLuint waterVelTex;
	
} MapBlock;





typedef struct MapBlockRenderInfo {
	int texIndex; // index into the origin offset texture map
	Vector2 originOffset; // offset from the view's origin at this time
	MapBlock* mb; // the map block this belongs to
	float minViewDistance; // distance of the min and max corners from the camera, 
	float maxViewDistance; //   in 3d
	float min2DDistance; // distance of min and max corners from the camera,
	float max2DDistance; //   projected onto the ground
	
	Matrix* mWorldView;
	Matrix* mViewProj;
} MapBlockRenderInfo;

struct sGL_RG8 {
	unsigned char x, y;
};



typedef struct MapRenderComponent {
	void (*alloc)(MapInfo*, MapBlock*, void*); // called to allocate memory for the block
	void (*create)(MapInfo*, MapBlock*, void*); // called once ever when the block is first created
		
	void (*load)(MapInfo*, MapBlock*, void*); 
	void (*save)(MapInfo*, MapBlock*, void*);
	
	void (*update)(MapInfo*, MapBlockRenderInfo*, void*);
	void (*draw)(MapInfo*, MapBlockRenderInfo*, void*);
} MapRenderComponent;


typedef struct MapInfo {
	
	// will be replaced by expandable structures later
	//TerrainBlock* tb;
	
	int w, h;
	
	MapBlock* block;
	//MapBlock* blocks[8][8];
	
	MultiDrawIndirect* blockPatch;

	
// 	MapBlockTreeLeaf* root;
	
	int scale; // how many terrain tiles are along an edge of one game tile. SC3k would be 1.
	
	//GLuint zoneColorTex;
	
	GLuint tex;
	GLuint terrainTex;
	GLuint locationTex;
	
	MapBlock* texIndexMap[64]; // indices align to gl tex array
	struct sGL_RG8* offsetData;
	int numBlocksToRender;
	
	MapBlock** blocksToRender;
	int blocksToRenderSz;
	int blocksToRenderCnt;
	
	
	// probably don'y need to keep these around
	unsigned int zoneColors[256];
	// probably needs to be somewhere else
	char* zoneNames[256];
	
	
	Vector cursorPos;
	TextureManager* tm;
	
} MapInfo;


typedef struct AreaStats {
	float min, max;
	float avg;
	float areaFlat; // area in x-y plane only
	float volume; // min is the base
	
} AreaStats;

void initMap(MapInfo* mi);
void initTerrain(MapInfo* mi);
void initTerrainBlock(MapBlock* mb, int cx, int cy);
MapBlock* allocMapBlock(int llx, int lly);
MapBlockTreeLeaf* allocMapBlockTreeLeaf(int llx, int lly);
MapBlock* spawnMapBlock(MapInfo* mi, int bix, int biy);
MapBlockTreeLeaf* spawnMapBlockTreeLeaf(MapInfo* mi, int llbix, int llbiy);

void updateTerrainTexture(MapInfo* mi);

void drawTerrainRoads(GLuint dtex, MapInfo* mi, Matrix* mView, Matrix* mProj, Vector* cursor, Vector2* viewWH);
void drawTerrain(MapInfo* mi, UniformBuffer* perViewUB, Vector* cursor, Vector2* viewWH);
void drawTerrainDepth(MapInfo* mi, UniformBuffer* perViewUB, Vector2* viewWH);
void checkMapDirty(MapInfo* mi);
void areaStats(TerrainBlock* tb, int x1, int y1, int x2, int y2, AreaStats* ass);

void flattenArea(TerrainBlock *tb, int x1, int y1, int x2, int y2);
void updateMapTextures(MapInfo* mi);
void setZone(MapInfo *mi, int x1, int y1, int x2, int y2, int zone);

float getTerrainHeightf(MapInfo* map, Vector2* coords);
void getTerrainHeight(MapInfo* map, Vector2i* coords, int coordLen, float* heightsOut);

void tileCenterWorld(MapInfo* map, int tx, int ty, Vector* out);


void saveMapBlock(FILE* f, MapBlock* mb);
MapBlock* loadMapBlock(FILE* f);

void MapInfo_initLayerTextures(MapInfo*mi);


void MapLayer_Fill(MapLayer* ml, float value);
void MapLayer_FillUChar(MapLayer* ml, unsigned char value);
int MapBlock_AddLayer(MapBlock* mb, char* name, int scale, char type);
MapLayer* MapBlock_GetLayer(MapBlock* mb, char* name);
void MapLayer_GenTerrain(MapLayer* ml);
MapLayer* MapLayer_Alloc(Vector2i size, float scale);
void MapLayer_init(MapLayer* ml, Vector2i size, float scale);
void MapInfo_Init(MapInfo* mi);
void MapInfo_GenMesh(MapInfo* mi);



void updateTerrainTexture(MapInfo* mi);


void Map_readConfigFile(MapInfo* map, char* path);

RenderPass* Map_CreateRenderPass(MapInfo* m); 
PassDrawable* Map_CreateDrawable(MapInfo* m);




float Map_getTerrainHeight(MapInfo* mi, Vector2i p);
float Map_getTerrainHeightf(MapInfo* mi, Vector2 p);
float Map_getTerrainHeight3f(MapInfo* mi, Vector p);

void Mapgen_v1(MapLayer* ml);
void MapGen_water(MapInfo* mb, ShaderProgram* prog);
void MapGen_erode(MapInfo* mb, ShaderProgram* prog);

void MapGen_initWaterVelTex(MapInfo* mi); 



#endif // __EACSMB_MAP_H__
