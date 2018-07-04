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
#include "staticMesh.h"
#include "dynamicMesh.h"
#include "waterPlane.h"
#include "pass.h"


struct GameState;

enum ItemTypes {
	ITEM_TYPE_UNKNOWN = 0,
	ITEM_TYPE_STATICMESH,
	ITEM_TYPE_DYNAMICMESH,
	ITEM_TYPE_EMITTER,
	ITEM_TYPE_LIGHT,
	ITEM_TYPE_DECAL,
	ITEM_TYPE_CUSTOMDECAL,
};


typedef struct {
	int type;
	int index;
	Vector offset;
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
} PartInstance;

typedef struct {
	Item* item;
	Vector pos;
	PartInstance parts[];
} ItemInstance;



struct OrientData {
	Vector pos;
	float scale;
	
	Vector dir;
	float alpha;
	
	Vector rotAxis;
	float rot;
	
};

struct ItemFlags {
	unsigned char shouldDelete : 1; 
	unsigned char wasDeleted : 1; 
	
};


typedef struct SimpleKinematics {
	Vector velocity;
	float angularVelocity;
	
	Vector acceleration;
	float angularAcceleration;
	
	Vector axisOfRotation;
	
} SimpleKinematics;



static const uint32_t ITEM_BASE_IDS[] = {
	[ITEM_TYPE_UNKNOWN] =     4000000000,
	[ITEM_TYPE_STATICMESH] =  0000000000,
	[ITEM_TYPE_DYNAMICMESH] = 1000000000,
	[ITEM_TYPE_EMITTER] =     1100000000,
	[ITEM_TYPE_LIGHT] =       1200000000,
	[ITEM_TYPE_DECAL] =       1300000000,
	[ITEM_TYPE_CUSTOMDECAL] = 1400000000,
};

static inline uint32_t itemBaseID(enum ItemTypes e) {
	return ITEM_BASE_IDS[e];
}


// World is the entire world's contents. Scene is the part you can see.
// eventually, probably, the graphics data will be moved into Scene

// no drawing is handled by the World code. It is for managment and operations only.
// drawing is too complicated and crosses too many lines to fit here.
typedef struct World {
	struct GameState* gs; // pointer to parent
	
	MeshManager* smm;
	DynamicMeshManager* dmm;
	Emitter* emitters;
	LightManager* lm;
	DecalManager* dm;
	CustomDecalManager* cdm;
	
	RenderPass* terrainPass; // temp hack
	RenderPass* solidsPass; // temp hack
	RenderPass* lightingPass; // temp hack
	RenderPass* decalPass; // temp hack
	
	TextureManager* mapTexMan;
	TextureManager* meshTexMan;
	TextureManager* decalTexMan;
	
	// old bezier roads
	//RoadBlock* roads;
	
	WaterPlane* wp;
	
	MapInfo map; 
	RoadNetwork* roads;
	
	VEC(Item*) items;
	HashTable(int) itemLookup;
	
	VEC(ItemInstance*) itemInstances;
	VEC(PartInstance*) partInstances;
	
	VEC(uint32_t) itemOrientKeys;
	
	VEC(struct OrientData) staticOrients[2];
	
	int curOrient;
	VEC(struct OrientData) orients[2];
	
	VEC(uint32_t) simpleKinematicsKeys;
	VEC(struct SimpleKinematics) simpleKinematics;
	
	
	VEC(struct ItemFlags) flags;
	
	
	
	PipeSegment testmesh;
	
	
} World;







void World_drawTerrain(World* w, PassFrameParams* pfp);
void World_drawSolids(World* w, PassFrameParams* pfp);
void World_drawDecals(World* w, PassFrameParams* pfp);

int World_spawnAt_Item(World* w, char* itemName, Vector* location);
int World_spawnAt_DynamicMesh(World* w, int dmIndex, Vector* location);
int World_spawnAt_StaticMesh(World* w, int smIndex, Vector* location);
int World_spawnAt_Light(World* w, int lightIndex, Vector* location); 
int World_spawnAt_Decal(World* w, int index, Vector* location);
int World_spawnAt_CustomDecal(World* w, int texIndex, float width, const Vector2* p1, const Vector2* p2);
void World_spawnAt_Road(World* w, Vector2* start,  Vector2* stop);
int World_spawnAt_Emitter(World* w, int emitterIndex, Vector* location);


void loadItemConfig(World* w, char* path);



void World_init(World* w);


#endif // __EACSMB_world_h__
