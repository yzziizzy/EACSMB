

#include "common_math.h"
#include "common_gl.h"

#include "utilities.h"


#include "game.h"
#include "world.h"
#include "scene.h"




void World_init(World* w) {
	
	VEC_INIT(&w->itemInstances);
	VEC_INIT(&w->partInstances);
	VEC_INIT(&w->items);
	
	// TODO: handle last frame's data on first frame
	VEC_INIT(&w->orients[0]);
	VEC_INIT(&w->orients[1]);
	w->curOrient = 0;
	
	w->lm = calloc(1, sizeof(*w->lm));
	LightManager_Init(w->lm);
	w->lightingPass = LightManager_CreateRenderPass(w->lm);
	// not static //w->lm->dtex = w->gs->depthTexBuffer;
	
	HT_init(&w->itemLookup, 4);
	
	initMap(&w->map);
	w->dmm = dynamicMeshManager_alloc(512);
	w->smm = meshManager_alloc();
	
	
	meshManager_readConfigFile(w->smm, "assets/config/models.json");
	dynamicMeshManager_readConfigFile(w->dmm, "assets/config/models.json");
	
	w->emitters = makeEmitter();
	
	loadItemConfig(w, "assets/config/items.json");
	
	meshManager_updateGeometry(w->smm);
	dynamicMeshManager_updateGeometry(w->dmm);
	
	StaticMeshInstance smi[] = {
		{
			{1,1,4}, 2.5,
			{1, 0, 0}, 0.0,
			.9, 0,0,0
		}
	};
	
//	meshManager_addInstance(w->smm, 0, &smi[0]);
//	meshManager_addInstance(w->smm, 0, &smi[1]);
//	meshManager_addInstance(w->smm, 0, &smi[2]);
//	meshManager_updateInstances(w->smm);
	
	
	
	
	
	// HACK: emitters 


	
	
	//  HACK roads
	w->roads = calloc(1, sizeof(*w->roads));
	initRoadBlock(w->roads);
	
	RoadControlPoint rcp = {
		{5,5},
		{50,50},
		{40,7}
	};
	int id;
	rbAddRoad(w->roads, &rcp, &id);
	
	roadblock_update_vbo(w->roads);
	
	// water plane, temporary hack
	w->wp = calloc(1, sizeof(*w->wp));
	WaterPlane_create(w->wp, 200, &(Vector){0,0,0});
	
	Pipe_init(&w->testmesh);
	
	// hack to test lightmanager
	LightManager_AddPointLight(w->lm, (Vector){10,10, 10}, 200, 20);
	
}


static Item* findItem(World* w, char* itemName) {
	int64_t index;
	Item* item;
	
	if(HT_get(&w->itemLookup, itemName, &index)) {
		fprintf(stderr, "!!! item not found: '%s'\n", itemName);
		return -1;
	}
	
	printf("index of %s:%d\n", itemName, index);
	return VEC_DATA(&w->items)[index];
}


static ItemInstance* allocItemInstance(Item* item) {
	ItemInstance* inst;
	
	inst = calloc(1, sizeof(*inst) + (sizeof(inst->parts[0]) * item->numParts));
	CHECK_OOM(inst);
	
	inst->item = item;
	
	return inst;
}

static int spawnPart(World* w, ItemPart* part, Vector* center) {
	Vector loc;
	
	vAdd(center, &part->offset, &loc);
	
	switch(part->type) {
		case ITEM_TYPE_DYNAMICMESH:
			return World_spawnAt_DynamicMesh(w, part->index, &loc);
		
		case ITEM_TYPE_STATICMESH:
			return World_spawnAt_StaticMesh(w, part->index, &loc);
		
		case ITEM_TYPE_EMITTER:
			return World_spawnAt_Emitter(w, part->index, &loc);

		case ITEM_TYPE_LIGHT:
			return World_spawnAt_Light(w, part->index, &loc);
	
		default:
			printf("unknown part item type: %d\n", part->type);
	}
}



int World_spawnAt_Item(World* w, char* itemName, Vector* location) {
	int i;
	Item* item;
	ItemInstance* inst;
	
	item = findItem(w, itemName);
	if(item < 0) {
		printf("returning \n");
		return 1;
	}
	
	
	inst = allocItemInstance(item);
	
	for(i = 0; i < item->numParts; i++) {
		//printf("trying to spawn %d : %d\n", item->parts[i].index, i);
		spawnPart(w, &item->parts[i], location);
		
	}
	
}


int World_spawnAt_DynamicMesh(World* w, int dmIndex, Vector* location) {
	DynamicMeshInstance dmi;
	float h;
	
	Vector2i loci;
	//printf("dynamic mesh spawn");
	loci.x = location->x;
	loci.y = location->y;
	
	// look up the height there.
	getTerrainHeight(&w->map, &loci, 1, &h);
	
	//printf("map h at [%.1f, %.1f]: %.4f\n", location->x, location->y, h);
	
	// spawn instance
	dmi.pos.x = location->x;
	dmi.pos.y = location->y;
	dmi.pos.z = h + .5; // HACK +.5z for gazebo origin offset
	
	dmi.scale = 1;
	
	dmi.dir = (Vector){1, 0, 0};
	dmi.rot = F_PI / 2.0;
	
	dmi.alpha = 1.0;
	
	dynamicMeshManager_addInstance(w->dmm, dmIndex, &dmi);
	// note: dynamic mesh instances are updated every frame automatically
}

int World_spawnAt_StaticMesh(World* w, int smIndex, Vector* location) {
	StaticMeshInstance smi;
	float h;
	
	Vector2i loci;
	
	//printf("static mesh spawn");
	loci.x = location->x;
	loci.y = location->y;
	
	// look up the height there.
	getTerrainHeight(&w->map, &loci, 1, &h);
	
	//printf("map h at [%.1f, %.1f]: %.4f\n", location->x, location->y, h);
	
	// spawn instance
	smi.pos.x = location->x;
	smi.pos.y = location->y;
	smi.pos.z = h + .5; // HACK +.5z for gazebo origin offset
	
	smi.scale = 1;
	
	smi.dir = (Vector){1, 0, 0};
	smi.rot = F_PI / 2.0;
	
	smi.alpha = 1.0;
	
	meshManager_addInstance(w->smm, smIndex, &smi);
	meshManager_updateInstances(w->smm);
	
}

int World_spawnAt_Emitter(World* w, int emitterIndex, Vector* location) {
	float h;
	
	Vector2i loci;
	
	loci.x = location->x;
	loci.y = location->y;
	
	// look up the height there.
	getTerrainHeight(&w->map, &loci, 1, &h);
	
	// printf("map h at [%.1f, %.1f]: %.4f\n", location->x, location->y, h);
	// emitter
	EmitterInstance inst = {
		.pos = (Vector){location->x, location->y, h + 2},
		.scale = 10,
		.start_time = 0,
		.lifespan = 1<<15
	};
	
	emitterAddInstance(w->emitters, &inst);
	emitter_update_vbo(w->emitters);
	
	
}

int World_spawnAt_Light(World* w, int lightIndex, Vector* location) {
	float h;
	
	Vector2i loci;
	Vector groundloc;
	
	loci.x = location->x;
	loci.y = location->y;
	// look up the height there.
	getTerrainHeight(&w->map, &loci, 1, &h);
	
	groundloc = (Vector){location->x, location->y, h};

	printf("spawning light %f,%f,%f\n", groundloc.x, groundloc.y, groundloc.z);

	
	LightManager_AddPointLight(w->lm, groundloc, 40.0, 0.02);
	
}



void World_spawnAt_Road(World* w, Vector2* start,  Vector2* stop) {
	
	RoadControlPoint rcp = {
		{start->x, start->y},
		{stop->x, stop->y},
		{(start->x + stop->x) / 2 , (stop->y + start->y) / 2}
	};
	int id;
	rbAddRoad(w->roads, &rcp, &id);
	
	roadblock_update_vbo(w->roads);
	
}



void World_drawTerrain(World* w) {
	drawTerrain(&w->map, &w->gs->perViewUB, &w->gs->cursorPos, &w->gs->screen.wh);
}



void World_drawSolids(World* w, Matrix* view, Matrix* proj) {
	//meshManager_draw(w->smm, view, proj);
	dynamicMeshManager_draw(w->dmm, view, proj);
	
	Draw_Emitter(w->emitters, view, proj, w->gs->frameTime);
	
	WaterPlane_draw(w->wp, view, proj);
}



void World_drawDecals(World* w, Matrix* view, Matrix* proj) {
	drawRoad(w->roads, w->gs->depthTexBuffer, view, proj);
}




void World_addSimpleKinematics(World* w, int itemNum) {
	
	SimpleKinematics sk;
	memset(&sk, 0, sizeof(sk));
	
	VEC_PUSH(&w->simpleKinematicsKeys, itemNum);
	VEC_PUSH(&w->simpleKinematics, sk);
	
} 























