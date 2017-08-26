#ifndef __EACSMB_world_h__
#define __EACSMB_world_h__


#include "ds.h"
#include "map.h"
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
	
	MapInfo map;
	
} World;








void World_drawSolid(World* w, Matrix* view, Matrix* proj);

int World_spawnAt_StaticMesh(World* w, int smIndex, Vector* location);







#endif // __EACSMB_world_h__
