#ifndef __EACSMB_world_h__
#define __EACSMB_world_h__


#include "ds.h"
#include "map.h"
#include "road.h"
#include "emitter.h"
#include "staticMesh.h"


struct GameState;


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
	
} World;







void World_drawTerrain(World* w);
void World_drawSolids(World* w, Matrix* view, Matrix* proj);
void World_drawDecals(World* w, Matrix* view, Matrix* proj);


int World_spawnAt_StaticMesh(World* w, int smIndex, Vector* location);
void World_spawnAt_Road(World* w, Vector2* start,  Vector2* stop);





#endif // __EACSMB_world_h__
