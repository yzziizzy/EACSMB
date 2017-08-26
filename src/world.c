

#include "common_math.h"

#include "world.h"
#include "scene.h"




void World_init(World* w) {
	
	initMap(&w->map);
	w->smm = meshManager_alloc();
	meshManager_readConfigFile(w->smm, "assets/config/models.json");
	
	meshManager_updateGeometry(w->smm);
	
	StaticMeshInstance smi[] = {
		{
			{1,1,16},
			{1, 0, 0},
			{.05, .05, .05 },
		},
		{
			{10,10,16},
			{1, 0, 0},
			{.05, .05, .05 },
		},
		{
			{20,20,16},
			{1, 0, 0},
			{.05, .05, .05 },
		},
	};
	
	meshManager_addInstance(w->smm, 0, &smi[0]);
	meshManager_addInstance(w->smm, 0, &smi[1]);
	meshManager_addInstance(w->smm, 0, &smi[2]);
	meshManager_updateInstances(w->smm);
	
}




int World_spawnAt_StaticMesh(World* w, int smIndex, Vector* location) {
	
	
	// look up the height there.
	
	// spawn instance
	
}





void World_drawSolid(World* w, Matrix* view, Matrix* proj) {
	meshManager_draw(w->smm, view, proj);
	
}



































