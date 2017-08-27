#ifndef __EACSMB_world_h__
#define __EACSMB_world_h__


#include "ds.h"
#include "map.h"
#include "road.h"
#include "emitter.h"
#include "staticMesh.h"


struct GameState;

enum {
	ITEM_TYPE_UNKNOWN = 0,
	ITEM_TYPE_STATICMESH,
	ITEM_TYPE_EMITTER
};


typedef struct {
	int type;
	int index;
	Vector offset;
} ItemPart;

typedef struct {
	char* name;
	int numParts;
	ItemPart* parts;
} Item;

// World is the entire world's contents. Scene is the part you can see.
// eventually, probably, the graphics data will be moved into Scene

// no drawing is handled by the World code. It is for managment and operations only.
// drawing is too complicated and crosses too many lines to fit here.
typedef struct World {
	struct GameState* gs; // pointer to parent
	
	MeshManager* smm;
	Emitter* emitters;
	
	RoadBlock* roads;
	
	MapInfo map;
	
	VEC(Item*) items;
	HashTable(int) itemLookup;
	
} World;







void World_drawTerrain(World* w);
void World_drawSolids(World* w, Matrix* view, Matrix* proj);
void World_drawDecals(World* w, Matrix* view, Matrix* proj);


int World_spawnAt_StaticMesh(World* w, int smIndex, Vector* location);
void World_spawnAt_Road(World* w, Vector2* start,  Vector2* stop);


void loadItemConfig(World* w, char* path);



#endif // __EACSMB_world_h__
