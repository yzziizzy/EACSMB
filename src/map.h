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

#include "pass.h"
#include "texture.h"

#include "road.h"
#include "view.h"
#include "uniformBuffer.h"


struct MapInfo;
typedef struct MapInfo MapInfo;

struct MapBlockTreeLeaf;
typedef struct MapBlockTreeLeaf MapBlockTreeLeaf;


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
	
	// old bezier roads
	//RoadBlock roads;
	
} MapBlock;

// typedef struct MapBlockTreeLeaf {
// 	MapBlock* c[8][8];
// 	int32_t minx, miny;
// } MapBlockTreeLeaf;
// 





// //TODO: mark some regular terrain tiles as transparent... to see excavations
// typedef struct VirtualTerrainBlock { // used for planning, renders as wireframe, usually
// 	float* zs;
// 	AABB2i worldBounds; // in world tile space, not TB tile space
// 	
// 	GLuint vbo;
// 	int vertices;
// 	
// 	TerrainBlock** blocks;
// 	
// } VirtualTerrainBlock;
// 

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

// typedef struct MapLayer_Roads {
// 	
// } MapLayer_Roads;
// 
// typedef struct MapLayer_Trees {
// 	TerrainBlock* blocks;
// } MapLayer_Trees;
// 
// typedef struct MapLayer_Zones {
// 	uint8_t* zone_data;
// 	// zone list
// 	// zone colors
// } MapLayer_Zones;
// 
// typedef struct MapLayer_Terrain {
// 	TerrainBlock* blocks;
// } MapLayer_Terrain;

// // just hardcode to some moderately large size for now
// typedef struct MapLayer {
// 	uint16_t id;
// 	uint16_t type;
// 	uint32_t flags;
// 	char* name;
// 	
// 	union {
// 		MapLayer_Terrain terrain;
// 		MapLayer_Roads roads;
// 		MapLayer_Zones zones;
// 	};
// } MapLayer;


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
	MapBlock* blocks[8][8];
	int blocksSz;
	int blocksLen;
// 	MapBlock* originMB;
	
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





void Map_readConfigFile(MapInfo* map, char* path);

RenderPass* Map_CreateRenderPass(MapInfo* m); 
PassDrawable* Map_CreateDrawable(MapInfo* m);







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
