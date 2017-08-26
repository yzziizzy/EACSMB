

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
			{1,1,4},
			{1, 0, 0},
			{2.5, 2.5, 2.5 },
		},
		{
			{5,5,4},
			{1, 0, 0},
			{2.05, 2.05, 2.05 },
		},
		{
			{20,20,45},
			{1, 0, 0},
			{2.01, .01, 2.01 },
		},
	};
	
	meshManager_addInstance(w->smm, 0, &smi[0]);
//	meshManager_addInstance(w->smm, 0, &smi[1]);
//	meshManager_addInstance(w->smm, 0, &smi[2]);
	meshManager_updateInstances(w->smm);
	
	
	
	
	// HACK: emitters 
	w->emitters = makeEmitter();
// 	EmitterInstance dust_instance = {
// 		.pos = {250.0,250.0,250.0},
// 		.scale = 10,
// 		.start_time = 0,
// 		.lifespan = 1<<15
// 	};
// 	
// 	emitterAddInstance(dust, &dust_instance);
// 	emitter_update_vbo(dust);
	
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
	smi.pos.z = h + .5; // HACK +.5z for gazebo origin offset
	
	smi.dir = (Vector){1, 0, 0};
	smi.scale = (Vector){1,1,1 };
	
	meshManager_addInstance(w->smm, smIndex, &smi);
	meshManager_updateInstances(w->smm);
	
	
	
	// emitter
	EmitterInstance dust_instance = {
		.pos = (Vector){location->x, location->y, h + 2},
		.scale = 10,
		.start_time = 0,
		.lifespan = 1<<15
	};
	
	emitterAddInstance(w->emitters, &dust_instance);
	emitter_update_vbo(w->emitters);
}





void World_drawTerrain(World* w) {
	drawTerrain(&w->gs->scene.map, &w->gs->perViewUB, &w->gs->cursorPos, &w->gs->screen.wh);
}



void World_drawSolids(World* w, Matrix* view, Matrix* proj) {
	meshManager_draw(w->smm, view, proj);
	
	Draw_Emitter(w->emitters, view, proj, w->gs->frameTime);
}



































