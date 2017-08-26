#ifndef __EACSMB_world_h__
#define __EACSMB_world_h__


#include "game.h"
#include "map.h"
#include "staticMesh.h"


// World is the entire world. Scene is the part you can see.

// no drawing is handled by the World code. It is for managment and operations only.
// drawing is too complicated and crosses too many lines to fit here.

typedef struct World {
	GameState* gs; // pointer to parent
	
	MeshManager* smm;
	Emitter* emitters;
	
	MapInfo map;
	
} World;

















#endif // __EACSMB_world_h__
