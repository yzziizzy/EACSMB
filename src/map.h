#ifndef __map_h__
#define __map_h__


// terrain textures must always be a power of two
#define TERR_TEX_SZ 1024
// terrain blocks must always be a power of two minus one
#define TERR_BLOCK_SZ (TERR_TEX_SZ - 2)

#define MAP_TEX_SZ (TERR_TEX_SZ - 1)



#define TCOORD(x,y) ((y * TERR_TEX_SZ) + x)
#define MCOORD(x,y) ((y * MAP_TEX_SZ) + x)


typedef struct MapBlock {
	unsigned char surface[MAP_TEX_SZ * MAP_TEX_SZ];
	unsigned char zones[MAP_TEX_SZ * MAP_TEX_SZ];
	
	AABB2i bounds; // in world tiles
	
	int dirtyZone, dirtySurface;
	
	GLuint tex;
	
} MapBlock;





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





//TODO: mark some regular terrain tiles as transparent... to see excavations
typedef struct VirtualTerrainBlock { // used for planning, renders as wireframe, usually
	float* zs;
	AABB2i worldBounds; // in world tile space, not TB tile space
	
	GLuint vbo;
	int vertices;
	
	TerrainBlock** blocks; 
	
} VirtualTerrainBlock;


typedef struct MapInfo {
	// will be replaced by expandable structures later
	TerrainBlock* tb;
	MapBlock* mb;
	
	
	int scale; // how many terrain tiles are along an edge of one game tile. SC3k would be 1.
	
	GLuint zoneColorTex;
	
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


void initTerrain(); 
TerrainBlock* allocTerrainBlock(int cx, int cy);
MapBlock* allocMapBlock(int llx, int lly);

void updateTerrainTexture(TerrainBlock* tb);

void drawTerrainBlock(MapInfo* tb, Matrix* mModel, Matrix* mView, Matrix* mProj, Vector2* cursor);
void checkMapDirty(MapInfo* mi);
void areaStats(TerrainBlock* tb, int x1, int y1, int x2, int y2, AreaStats* ass);

void flattenArea(TerrainBlock *tb, int x1, int y1, int x2, int y2);
void updateMapTextures(MapBlock* mb);
void setZone(MapInfo *mi, int x1, int y1, int x2, int y2, int zone);

void getTerrainHeight(MapInfo* map, Vector2i* coords, int coordLen, float* heightsOut); 



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

#endif // __map_h__