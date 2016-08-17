#ifndef __EACSMB_MAP_H__
#define __EACSMB_MAP_H__


// terrain textures must always be a power of two
#define TERR_TEX_SZ 256
// terrain blocks must always be a power of two minus one
#define TERR_BLOCK_SZ (TERR_TEX_SZ)

#define MAP_TEX_SZ (TERR_TEX_SZ)



#define TCOORD(x,y) ((y * TERR_TEX_SZ) + x)
#define MCOORD(x,y) ((y * MAP_TEX_SZ) + x)

#include "road.h"

typedef struct TerrainPatchVertex {
	float x, y, z;
	float hmU, hmV; // these are in texels
	float divX, divY;
} TerrainPatchVertex;


typedef struct TerrainBlock {
	float zs[TERR_TEX_SZ * TERR_TEX_SZ];
	int cx, cy;
	AABB2 box;
	int scale;
	
	int dirty;
	AABB2 dirtyBox;
	
	GLuint tex;
	
} TerrainBlock;




typedef struct MapBlock {
	int32_t bix, biy; // location in blocks. max world size is 2^32 blocks square ~5.6e8 km2 ~1.1x earth surface area  
	
	struct MapBlock* n_xp, *n_xm, *n_yp, *n_ym;
	
	unsigned char surface[TERR_TEX_SZ * TERR_TEX_SZ];
	unsigned char zones[TERR_TEX_SZ * TERR_TEX_SZ];
	
	int dirtyZone, dirtySurface;
	

	TerrainBlock tb;
	
	RoadBlock roads;
	
} MapBlock;

typedef struct MapBlockTreeLeaf {
	MapBlock* c[8][8];
	int32_t minx, miny;
} MapBlockTreeLeaf;






//TODO: mark some regular terrain tiles as transparent... to see excavations
typedef struct VirtualTerrainBlock { // used for planning, renders as wireframe, usually
	float* zs;
	AABB2i worldBounds; // in world tile space, not TB tile space
	
	GLuint vbo;
	int vertices;
	
	TerrainBlock** blocks;
	
} VirtualTerrainBlock;


/*

in general:

render opaque buildings
render terrain
render infrastructure
render ui



conservative frustum culling/selection
sort by distance
render front to back


*/

struct sGL_RG8 {
	unsigned char x, y;
};


typedef struct MapInfo {
	// will be replaced by expandable structures later
	//TerrainBlock* tb;
	MapBlock** blocks;
	int blocksSz;
	int blocksLen;
	MapBlock* originMB;
	
	MapBlockTreeLeaf* root;
	
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
} MapInfo;


typedef struct AreaStats {
	float min, max;
	float avg;
	float areaFlat; // area in x-y plane only
	float volume; // min is the base
	
} AreaStats;


void initTerrain(MapInfo* mi);
void initTerrainBlock(MapBlock* mb, int cx, int cy);
MapBlock* allocMapBlock(int llx, int lly);
MapBlockTreeLeaf* allocMapBlockTreeLeaf(int llx, int lly);
MapBlock* spawnMapBlock(MapInfo* mi, int bix, int biy);
MapBlockTreeLeaf* spawnMapBlockTreeLeaf(MapInfo* mi, int llbix, int llbiy);

void updateTerrainTexture(MapInfo* mi);

void drawTerrainRoads(GLuint dtex, MapInfo* mi, Matrix* mView, Matrix* mProj, Vector* cursor, Vector2* viewWH);
void drawTerrain(MapInfo* mi, Matrix* mView, Matrix* mProj, Vector* cursor, Vector2* viewWH);
void drawTerrainDepth(MapInfo* mi, Matrix* mView, Matrix* mProj, Vector2* viewWH);
void checkMapDirty(MapInfo* mi);
void areaStats(TerrainBlock* tb, int x1, int y1, int x2, int y2, AreaStats* ass);

void flattenArea(TerrainBlock *tb, int x1, int y1, int x2, int y2);
void updateMapTextures(MapInfo* mi);
void setZone(MapInfo *mi, int x1, int y1, int x2, int y2, int zone);

void getTerrainHeight(MapInfo* map, Vector2i* coords, int coordLen, float* heightsOut);


void saveMapBlockTreeLeaf(FILE* f, MapBlockTreeLeaf* mbl);
MapBlockTreeLeaf* loadMapBlockTreeLeaf(FILE* f);
// stuff below is too complicated for now. more knowledge is needed about the game to proceed.


/*
#define MAP_SZ 32


#define MAP_IDX_MASK(x,y) (((1 << 2) << (x)) & (1 << (y)))

struct MapBlock;



typedef struct MapInfo {
	short dataStride;
	
	MapNode root;
}



typedef struct MapNode {
	
	
	unsigned char level;
	unsigned char usage;
	
	int x, y; // center coords
	struct MapNode* parent;
	
	union {
		struct MapNode* kids[2][2];
		struct MapBlock* blocks[2][2];
	};
} MapNode;


typedef struct MapBlock {
	void* data;
	int cx, cy;
	int counter;
	struct MapBlock* xp, *xm, *yp, *ym;
} MapBlock;




// MapBlock* allocRootMapNode(short stride);







void printMapMemoryStats();

*/

#endif // __EACSMB_MAP_H__
