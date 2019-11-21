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
#include "dynamicMesh.h"
#include "marker.h"
#include "bushes.h"
#include "waterPlane.h"
#include "sound.h"
#include "pass.h"
#include "shadowMap.h"

#include "items.h"

struct GameState;





typedef struct SceneItemInfo {
	AABB2 aabb;
	
	uint32_t selectable : 1;
	
	uint32_t eid;
	void* data;
} SceneItemInfo;


typedef struct QuadTreeNode {
	AABB2 aabb;
	int level;
	VEC(SceneItemInfo*) items;
	
	struct QuadTreeNode* parent;
	struct QuadTreeNode* kids[2][2];
} QuadTreeNode;


typedef struct QuadTree {
	int maxLevels;
	int nodeMaxCount; // trigger to subdivide
	int nodeMinCount; // trigger to recombine // ignored for now
	
	int totalCount;
	int totalNodes;
	
	MemPool siPool;
	
	QuadTreeNode* root;
} QuadTree;


typedef int (*QuadTreeFindFn)(SceneItemInfo*, void*);

void QuadTree_init(QuadTree* qt, AABB2 bounds);
void QuadTree_insert(QuadTree* qt, SceneItemInfo* siNew);
void QuadTree_purge(QuadTree* qt, SceneItemInfo* siDead);
SceneItemInfo* QuadTree_findFirst(QuadTree* qt, Vector2 pt);
void QuadTree_findAll(QuadTree* qt, Vector2 pt, QuadTreeFindFn fn, void* data);
void QuadTree_findAllArea(QuadTree* qt, AABB2 aabb, QuadTreeFindFn fn, void* data);
SceneItemInfo* QuadTree_allocSceneItem(QuadTree* qt);

// temp
 int QuadTreeNode_split(QuadTree* qt, QuadTreeNode* n);


void QuadTree_renderDebugVolumes(QuadTree* qt, PassFrameParams* pfp);


static const uint32_t ITEM_BASE_IDS[] = {
	[PART_TYPE_UNKNOWN] =     4000000000,
// 	[PART_TYPE_STATICMESH] =  0000000000,
	[PART_TYPE_DYNAMICMESH] = 1000000000,
	[PART_TYPE_EMITTER] =     1100000000,
	[PART_TYPE_LIGHT] =       1200000000,
	[PART_TYPE_DECAL] =       1300000000,
	[PART_TYPE_CUSTOMDECAL] = 1400000000,
	[PART_TYPE_MARKER] =      1500000000,
};

static inline uint32_t itemBaseID(enum PartType e) {
	return ITEM_BASE_IDS[e];
}

#define EditCmd_Empty_ID 0
#define EditCmd_Spawn_ID 1
#define EditCmd_Move_ID 2


typedef struct EditCmd_Empty {
	uint32_t cmdID;
	
	uint32_t is_terminator;
} EditCmd_Empty;

// spawns relative to the ground level
typedef struct EditCmd_Spawn {
	uint32_t cmdID;
	
	uint32_t type;
	char* name;
	
	Vector pos;
} EditCmd_Spawn;

typedef struct EditCmd_Move{
	uint32_t cmdID;
	
	uint32_t type;
	uint32_t id;
	
	Vector pos;
} EditCmd_Move;

typedef union EditCmd{
	EditCmd_Empty Empty;
	EditCmd_Spawn Spawn;
	EditCmd_Move Move;
} EditCmd;



typedef struct World {
	struct GameState* gs; // pointer to parent
	
	//MeshManager* smm; // OBSOLETE: use DynamicMeshManager
	DynamicMeshManager* dmm;
	MarkerManager* mm;
	EmitterManager* em;
	LightManager* lm;
	DecalManager* dm;
	CustomDecalManager* cdm;
	BushManager* bushm;
	
	RenderPass* terrainPass; // temp hackMap_CreateDrawable(m);
	RenderPass* solidsPass; // temp hack
	RenderPass* bushesPass; // temp hack
	RenderPass* transparentsPass; // temp hack
	RenderPass* lightingPass; // temp hack
	RenderPass* decalPass; // temp hack
	RenderPass* emitterPass; // temp hack
	
	
	TextureManager* mapTexMan; // color and normals - RGB
	TextureManager* mapMatTexMan; // R
	
	TextureManager* meshTexMan; // RGBA
	TextureManager* meshNormTexMan; // RGB
	TextureManager* meshMatTexMan; // R
	
	TextureManager* decalTexMan;
	TextureManager* emitterTexMan;
	
	
	// old bezier roads
	//RoadBlock* roads;
	
	// HACK: should be moved elsewhere
	ShadowMap* sunShadow;
	
	WaterPlane* wp;
	
	QuadTree qt;
	MapInfo map; 
	RoadNetwork* roads;
	
	VEC(Item*) items;
	HashTable(int) itemLookup;

	VEC(Part) parts;
	HashTable(int) partLookup;
	
	VEC(ItemInstance*) itemInstances;
	VEC(PartInstance*) partInstances;
	
	CustomDecalInstance* cursor;
	
	#include "../mods/World.generated_mixin.h" 
	
} World;





void World_loadItemConfigFileNew(World* w, char* path);
void World_loadItemConfigNew(World* w, json_value_t* jo);

void World_drawTerrain(World* w, PassFrameParams* pfp);
void World_drawSolids(World* w, PassFrameParams* pfp);
void World_drawLeaves(World* w, PassFrameParams* pfp); // alpha-stenciled
void World_preTransparents(World* w, PassFrameParams* pfp);
void World_drawTransparents(World* w, PassFrameParams* pfp);
void World_postTransparents(World* w);
void World_drawDecals(World* w, PassFrameParams* pfp);


int World_lookUp_Item(World* w, char* name);
int World_lookUp_SubItem(World* w, enum ItemType type, char* name);

int World_spawnAt_Item(World* w, char* itemName, Vector* location);
int World_spawnAt_ItemPtr(World* w, Item* item, Vector* location);
int World_spawnAt_DynamicMesh(World* w, int dmIndex, Vector* location);
int World_spawnAt_Light(World* w, int lightIndex, Vector* location); 
int World_spawnAt_Decal(World* w, int index, Vector* location);
int World_spawnAt_CustomDecal(World* w, int cdecalIndex, float width, const Vector2* p1, const Vector2* p2);
void World_spawnAt_Road(World* w, Vector2* start,  Vector2* stop);
int World_spawnAt_Emitter(World* w, int emitterIndex, Vector* location);
int World_spawnAt_Marker(World* w, int markerIndex, Vector* location);


void loadItemConfig(World* w, char* path);



void World_init(World* w);
void World_initGL(World* w);


#endif // __EACSMB_world_h__
