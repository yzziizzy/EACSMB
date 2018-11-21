#ifndef __EACSMB_world_h__
#define __EACSMB_world_h__


#include "ds.h"
#include "map.h"
#include "road.h"
#include "pipe.h"
#include "emitter.h"
#include "lighting.h"
#include "decals.h"
#include "decalsCustom.h"
// #include "staticMesh.h"
#include "dynamicMesh.h"
#include "marker.h"
#include "waterPlane.h"
#include "pass.h"
#include "shadowMap.h"


struct GameState;

enum ItemTypes {
	ITEM_TYPE_UNKNOWN = 0,
	ITEM_TYPE_ITEM, // for parsing
	ITEM_TYPE_STATICMESH,
	ITEM_TYPE_DYNAMICMESH,
	ITEM_TYPE_EMITTER,
	ITEM_TYPE_LIGHT,
	ITEM_TYPE_DECAL,
	ITEM_TYPE_CUSTOMDECAL,
	ITEM_TYPE_MARKER,
};


typedef struct { // information about the part itself
	enum ItemTypes type;
	int index;
	char* name;
} Part;


typedef struct { // info about how the part relates to the item
	enum ItemTypes type;
	int index;
	int partIndex;
	Vector offset; // rotation, scale, etc
	//void* data;
} ItemPart;

typedef struct {
	char* name;
	int numParts;
	ItemPart* parts;
	
	
	
} Item;



typedef struct {
	ItemPart* part;
	int parentItemInst;
	uint32_t eid;
} PartInstance;

typedef struct {
	Item* item;
	Vector pos;
	
	uint32_t eid;
	int numParts;
	
	PartInstance parts[];
} ItemInstance;








static const uint32_t ITEM_BASE_IDS[] = {
	[ITEM_TYPE_UNKNOWN] =     4000000000,
	[ITEM_TYPE_STATICMESH] =  0000000000,
	[ITEM_TYPE_DYNAMICMESH] = 1000000000,
	[ITEM_TYPE_EMITTER] =     1100000000,
	[ITEM_TYPE_LIGHT] =       1200000000,
	[ITEM_TYPE_DECAL] =       1300000000,
	[ITEM_TYPE_CUSTOMDECAL] = 1400000000,
	[ITEM_TYPE_MARKER] =      1500000000,
};

static inline uint32_t itemBaseID(enum ItemTypes e) {
	return ITEM_BASE_IDS[e];
}



typedef struct World {
	struct GameState* gs; // pointer to parent
	
	//MeshManager* smm; // OBSOLETE: use DynamicMeshManager
	DynamicMeshManager* dmm;
	MarkerManager* mm;
	Emitter* emitters;
	LightManager* lm;
	DecalManager* dm;
	CustomDecalManager* cdm;
	
	RenderPass* terrainPass; // temp hackMap_CreateDrawable(m);
	RenderPass* solidsPass; // temp hack
	RenderPass* transparentsPass; // temp hack
	RenderPass* lightingPass; // temp hack
	RenderPass* decalPass; // temp hack
	
	RenderPass* terrainSelectionPass; // temp hack
	
	TextureManager* mapTexMan;
	TextureManager* meshTexMan;
	TextureManager* decalTexMan;
	
	// old bezier roads
	//RoadBlock* roads;
	
	// HACK: should be moved elsewhere
	ShadowMap* sunShadow;
	
	WaterPlane* wp;
	
	MapInfo map; 
	RoadNetwork* roads;
	
	VEC(Item*) items;
	HashTable(int) itemLookup;

	VEC(Part) parts;
	HashTable(int) partLookup;
	
	VEC(ItemInstance*) itemInstances;
	VEC(PartInstance*) partInstances;
	

	PipeSegment testmesh;
	
	
} World;





void World_loadItemConfigFileNew(World* w, char* path);
void World_loadItemConfigNew(World* w, json_value_t* jo);

void World_drawTerrain(World* w, PassFrameParams* pfp);
void World_drawSolids(World* w, PassFrameParams* pfp);
void World_preTransparents(World* w, PassFrameParams* pfp);
void World_drawTransparents(World* w, PassFrameParams* pfp);
void World_postTransparents(World* w);
void World_drawDecals(World* w, PassFrameParams* pfp);


int World_lookUp_Item(World* w, char* name);
int World_lookUp_SubItem(World* w, enum ItemType type, char* name);

int World_spawnAt_Item(World* w, char* itemName, Vector* location);
int World_spawnAt_DynamicMesh(World* w, int dmIndex, Vector* location);
// int World_spawnAt_StaticMesh(World* w, int smIndex, Vector* location);
int World_spawnAt_Light(World* w, int lightIndex, Vector* location); 
int World_spawnAt_Decal(World* w, int index, Vector* location);
int World_spawnAt_CustomDecal(World* w, int texIndex, float width, const Vector2* p1, const Vector2* p2);
void World_spawnAt_Road(World* w, Vector2* start,  Vector2* stop);
int World_spawnAt_Emitter(World* w, int emitterIndex, Vector* location);
int World_spawnAt_Marker(World* w, int markerIndex, Vector* location);


void loadItemConfig(World* w, char* path);



void World_init(World* w);


#endif // __EACSMB_world_h__
