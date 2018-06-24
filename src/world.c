

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
	
	w->meshTexMan = TextureManager_alloc();
	w->decalTexMan = TextureManager_alloc();
	
	w->dmm = dynamicMeshManager_alloc(1024*50);
	w->smm = meshManager_alloc();
	w->dm = DecalManager_alloc(1024*50);
	w->cdm = CustomDecalManager_alloc(1024*50);
	
	w->dmm->tm = w->meshTexMan;
	w->dm->tm = w->decalTexMan;
	w->cdm->tm = w->decalTexMan;
	
	meshManager_readConfigFile(w->smm, "assets/config/models.json");
	dynamicMeshManager_readConfigFile(w->dmm, "assets/config/models.json");
	DecalManager_readConfigFile(w->dm, "assets/config/decals.json");
	
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
	
	
	// very last thing: load textures
	TextureManager_loadAll(w->meshTexMan, (Vector2i){128, 128}); 
	TextureManager_loadAll(w->decalTexMan, (Vector2i){128, 128}); 

	w->decalPass = DecalManager_CreateRenderPass(w->dm);
	printf("world dm texman id: %d\n", w->dm->tm->tex_id);
	
	RenderPass_addDrawable(w->decalPass, CustomDecalManager_CreateDrawable(w->cdm));
	
	for(int i = 0; i < 5000; i++) {
		Vector v = {
			.x = frand(0, 1000),
			.y = frand(0, 1000),
			.z = 20,
		};
		
		World_spawnAt_Item(w, "tree", &v);
	}
	
	CustomDecal* cd = pcalloc(cd); 
	cd->thickness = 9.0f;
	
	CustomDecalManager_AddDecal(w->cdm, "test", cd);
	
	Matrix rm;
	
	mIdent(&rm);
	mTrans3f(100, 100, 0, &rm);
	mRotZ(1, &rm);

	Vector pos1 = {40, 55, 20};
	Vector pos2 = {40, 95, 20};
	Vector pos3 = {100, 30, 20};
	Vector pos4 = {90, 110, 20};
	
	vMatrixMul(&pos1, &rm, &pos1);
	vMatrixMul(&pos2, &rm, &pos2);
	vMatrixMul(&pos3, &rm, &pos3);
	vMatrixMul(&pos4, &rm, &pos4);
	
	CustomDecalManager_AddInstance(w->cdm, 0, &(CustomDecalInstance){
		.pos1 = pos1,
		.pos2 = pos2,
		.pos3 = pos3,
		.pos4 = pos4,
		.thickness = 50,
	});
}


static Item* findItem(World* w, char* itemName) {
	int64_t index;
	Item* item;
	
	if(HT_get(&w->itemLookup, itemName, &index)) {
		fprintf(stderr, "!!! item not found: '%s'\n", itemName);
		return -1;
	}
	
	//printf("index of %s:%d\n", itemName, index);
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

		case ITEM_TYPE_DECAL:
			return World_spawnAt_Decal(w, part->index, &loc);
	
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

	//printf("spawning light %f,%f,%f\n", groundloc.x, groundloc.y, groundloc.z);

	
	LightManager_AddPointLight(w->lm, groundloc, 10.0, 0.02);
	
}

int World_spawnAt_Decal(World* w, int index, Vector* location) {
	float h;
	
	Vector2i loci;
	Vector groundloc;
	
	loci.x = location->x;
	loci.y = location->y;
	// look up the height there.
	getTerrainHeight(&w->map, &loci, 1, &h);
	
	groundloc = (Vector){location->x, location->y, h};

	//printf("spawning decal %f,%f,%f\n", groundloc.x, groundloc.y, groundloc.z);

	DecalInstance di;
	
	di.pos = groundloc;
	
	DecalManager_AddInstance(w->dm, index, &di);
	
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



void World_drawSolids(World* w, PassFrameParams* pfp) {
	//meshManager_draw(w->smm, view, proj);
	dynamicMeshManager_draw(w->dmm, pfp);
	
	Draw_Emitter(w->emitters, pfp->dp->mWorldView, pfp->dp->mViewProj, w->gs->frameTime);
	
	WaterPlane_draw(w->wp, pfp->dp->mWorldView, pfp->dp->mViewProj);
}



void World_drawDecals(World* w, PassFrameParams* pfp) {
	drawRoad(w->roads, w->gs->depthTexBuffer, pfp->dp->mWorldView, pfp->dp->mViewProj);
	
	RenderPass_preFrameAll(w->decalPass, pfp);
	RenderPass_renderAll(w->decalPass, pfp->dp);
	RenderPass_postFrameAll(w->decalPass);
	
}




void World_addSimpleKinematics(World* w, int itemNum) {
	
	SimpleKinematics sk;
	memset(&sk, 0, sizeof(sk));
	
	VEC_PUSH(&w->simpleKinematicsKeys, itemNum);
	VEC_PUSH(&w->simpleKinematics, sk);
	
} 























