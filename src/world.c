

#include "common_math.h"

#include "game.h"
#include "world.h"
#include "scene.h"




void World_init(World* w) {
	
	initMap(&w->map);
	w->smm = meshManager_alloc();
	meshManager_readConfigFile(w->smm, "assets/config/models.json");
	
	meshManager_updateGeometry(w->smm);
	
	StaticMeshInstance smi[] = {
		{
			{1,1,45.21},
			{1, 0, 0},
			{.005, .005, .005 },
		},
		{
			{5,5,46.56},
			{1, 0, 0},
			{.005, .005, .005 },
		},
		{
			{20,20,45},
			{1, 0, 0},
			{.01, .01, .01 },
		},
	};
	
	meshManager_addInstance(w->smm, 0, &smi[0]);
	meshManager_addInstance(w->smm, 0, &smi[1]);
	meshManager_addInstance(w->smm, 0, &smi[2]);
	meshManager_updateInstances(w->smm);
	
}




int World_spawnAt_StaticMesh(World* w, int smIndex, Vector* location) {
	StaticMeshInstance smi;
	float h;
	
	Vector2i loci;
	
	loci.x = location->x;
	loci.y = location->y;
	
	// look up the height there.
	getTerrainHeight(&w->map, &loci, 1, &h);
	
	printf("map h at [%.1f, %.1f]: %.4f\n", location->x, location->y, h);
	
	// spawn instance
	smi.pos.x = location->x;
	smi.pos.y = location->y;
	smi.pos.z = h;
	
	smi.dir = (Vector){1, 0, 0};
	smi.scale = (Vector){.005, .005, .005 };
	
	meshManager_addInstance(w->smm, smIndex, &smi);
	meshManager_updateInstances(w->smm);
}





void World_drawTerrain(World* w) {
	drawTerrain(&w->gs->scene.map, &w->gs->perViewUB, &w->gs->cursorPos, &w->gs->screen.wh);
}



void World_drawSolids(World* w, Matrix* view, Matrix* proj) {
	meshManager_draw(w->smm, view, proj);
	
}



































