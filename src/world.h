#ifndef __EACSMB_world_h__
#define __EACSMB_world_h__


#include "map.h"
#include "staticMesh.h"


struct GameState;


// World is the entire world. Scene is the part you can see.

// no drawing is handled by the World code. It is for managment and operations only.
// drawing is too complicated and crosses too many lines to fit here.
typedef struct World {
	struct GameState* gs; // pointer to parent
	
	MeshManager* smm;
	Emitter* emitters;
	
	MapInfo map;
	
} World;








void World_drawSolid(World* w, Matrix* view, Matrix* proj);








#endif // __EACSMB_world_h__
