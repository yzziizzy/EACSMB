

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
	
	HT_init(&w->itemLookup, 4);
	
	initMap(&w->map);
	w->smm = meshManager_alloc();
	
	
	meshManager_readConfigFile(w->smm, "assets/config/models.json");
	
	w->emitters = makeEmitter();
	
	loadItemConfig(w, "assets/config/items.json");
	
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
	
	
	Pipe_init(&w->testmesh);
	
}


static Item* findItem(World* w, char* itemName) {
	int64_t index;
	Item* item;
	
	if(HT_get(&w->itemLookup, itemName, &index)) {
		fprintf(stderr, "!!! item not found: '%s'\n", itemName);
		return -1;
	}
	
	printf("index :%d\n", index);
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
		case ITEM_TYPE_STATICMESH:
			return World_spawnAt_StaticMesh(w, part->index, &loc);
		
		case ITEM_TYPE_EMITTER:
			return World_spawnAt_Emitter(w, part->index, &loc);
	
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
		
		spawnPart(w, &item->parts[i], location);
		
	}
	
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
	
}

int World_spawnAt_Emitter(World* w, int emitterIndex, Vector* location) {
	float h;
	
	Vector2i loci;
	
	loci.x = location->x;
	loci.y = location->y;
	
	// look up the height there.
	getTerrainHeight(&w->map, &loci, 1, &h);
	
	printf("map h at [%.1f, %.1f]: %.4f\n", location->x, location->y, h);
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
	drawTerrain(&w->gs->scene.map, &w->gs->perViewUB, &w->gs->cursorPos, &w->gs->screen.wh);
}



void World_drawSolids(World* w, Matrix* view, Matrix* proj) {
	meshManager_draw(w->smm, view, proj);
	
	Draw_Emitter(w->emitters, view, proj, w->gs->frameTime);
}



void World_drawDecals(World* w, Matrix* view, Matrix* proj) {
	drawRoad(w->roads, w->gs->depthTexBuffer, view, proj);
}



























