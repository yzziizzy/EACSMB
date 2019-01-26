

#include "common_math.h"
#include "common_gl.h"

#include "utilities.h"


#include "game.h"
#include "world.h"
#include "scene.h"

// temp?
#include "building.h"

#include "perlin.h"




void World_init(World* w) {
	VEC_INIT(&w->itemInstances);
	VEC_INIT(&w->partInstances);
	VEC_INIT(&w->items);
	VEC_INIT(&w->parts);
	
	HT_init(&w->itemLookup, 4);
	HT_init(&w->partLookup, 4);
	
	MapInfo_Init(&w->map, &w->gs->globalSettings);
	
	QuadTree_init(&w->qt, (AABB2){{0,0}, {512, 512}});
	
	w->lm = calloc(1, sizeof(*w->lm));
	
	w->dmm = dynamicMeshManager_alloc(&w->gs->globalSettings);
	w->dm = DecalManager_alloc(&w->gs->globalSettings);
	w->cdm = CustomDecalManager_alloc(&w->gs->globalSettings);
	w->mm = MarkerManager_alloc(&w->gs->globalSettings);
	w->em = EmitterManager_alloc(&w->gs->globalSettings);
	
	// hack becore CES is made
	w->dmm->ces = &w->gs->ces;
	
	w->mapTexMan = TextureManager_alloc();
	w->meshTexMan = TextureManager_alloc();
	w->decalTexMan = TextureManager_alloc();
	w->emitterTexMan = TextureManager_alloc();
	
	w->map.tm = w->mapTexMan;
	w->dmm->tm = w->meshTexMan;
	w->mm->tm = w->decalTexMan;
	w->dm->tm = w->decalTexMan;
	w->cdm->tm = w->decalTexMan;
	w->em->tm = w->emitterTexMan;
	
	
	World_loadItemConfigFileNew(w, w->gs->globalSettings.worldConfigPath);
	
	
	
	
	
	
	
	// -------- building test ------------
	Building b;
	VEC_INIT(&b.outlines);
	VEC_INIT(&b.vertices);
	VEC_INIT(&b.indices);
	
	
	BuildingOutline* bo;
	bo = BuildingOutline_rect(0, 30, (Vector2){0,0}, (Vector2){10,10});
	VEC_PUSH(&b.outlines, bo);
	bo = BuildingOutline_rect(0, 20, (Vector2){-6,0}, (Vector2){2,10});
	VEC_PUSH(&b.outlines, bo);
	bo = BuildingOutline_rect(0, 20, (Vector2){6,0}, (Vector2){2,10});
	VEC_PUSH(&b.outlines, bo);
	bo = BuildingOutline_rect(0, 10, (Vector2){-8,0}, (Vector2){2,10});
	VEC_PUSH(&b.outlines, bo);
	bo = BuildingOutline_rect(0, 10, (Vector2){8,0}, (Vector2){2,10});
	VEC_PUSH(&b.outlines, bo);
	
	Building_extrudeAll(&b);
	
// 	VEC_EACH(&b.vertices, i, v) {
// 		printf("~%d [%.2f,%.2f,%.2f]\n", i, v.p.x,v.p.y,v.p.z);
// 	}
	
	Building_capAll(&b); // causes memory corruption -- fixed?
	
	
	int building_ind = dynamicMeshManager_addMesh(w->dmm, "building", Building_CreateDynamicMesh(&b));
	
	DynamicMeshInstance inst = {
		pos: {100, 100, 30},
		scale: 30,
		dir: {0, 0, 1},
		rot: 0,
		alpha: 0.5,
		texIndex: 2,
	};
	//dynamicMeshManager_addInstance(w->dmm, building_ind, &inst);
	//printf("^^^^ %d\n", building_ind);
	Vector v = {50,50,0};
	
	
	//World_spawnAt_DynamicMesh(w, building_ind, &v);
	
	// -----------------------------------
	
	w->roads = RoadNetwork_alloc();
	
	
	
	
	
	int nn = 0;
	for(int y = 2; y < 510; y+=2) {
		for(int x = 2; x < 510; x+=2) {
			Vector v = {
				.x = x + frand(-2, 2),
				.y = y + frand(-2, 2),
				.z = 0,
			};
			
			float f = fabs(PerlinNoise_2D((0 + x) / 512.0, (0 + y) / 512.0, .2, 6));
			
// 			if(nn > 4) goto DONE;
			//printf("f = %f\n", f);
// 			if(f < -0.01) continue; 
// 			if(frandNorm() < .5) continue;
			if(f / frandNorm() < 1.8) continue;
			
			World_spawnAt_Item(w, "tree", &v);
			nn++;
			
		}
	}
	
	
	
 	CustomDecal* cd;

	DONE:

 	cd = pcalloc(cd); 
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
	
	w->cursor = CustomDecalManager_AddInstance(w->cdm, 0, &(CustomDecalInstance){
		.pos1 = pos1,
		.pos2 = pos2,
		.pos3 = pos3,
		.pos4 = pos4,
		.thickness = 5,
		.tex12 = .5,
		.tex34 = 2,
	});
 	
	
	
// 	World_spawnAt_CustomDecal(w, 0, 1, &(Vector2){100, 100}, &(Vector2){300, 300});
	
	#include "../mods/World_init.generated_thunk.c" 
	
}

void World_initGL(World* w) {
	
	
	LightManager_Init(w->lm);
	w->lightingPass = LightManager_CreateRenderPass(w->lm);
	// not static //w->lm->dtex = w->gs->depthTexBuffer;

	
	//initMap(&w->map);
	MapInfo_InitGL(&w->map, &w->gs->globalSettings);
	

	
	dynamicMeshManager_initGL(w->dmm, &w->gs->globalSettings);
	DecalManager_initGL(w->dm, &w->gs->globalSettings);
	CustomDecalManager_initGL(w->cdm, &w->gs->globalSettings);
	MarkerManager_initGL(w->mm, &w->gs->globalSettings);
	EmitterManager_initGL(w->em, &w->gs->globalSettings);


	
	float shadSz = w->gs->globalSettings.SunShadow_size;
	w->sunShadow = ShadowMap_alloc();
	w->sunShadow->size = (Vector2i){shadSz, shadSz};
	ShadowMap_SetupFBOs(w->sunShadow);

	

	
/*
	World_loadItemConfigFileNew(w, "assets/config/combined_config.json");
	*/
	Map_readConfigFile(&w->map, "assets/config/terrain.json");
	
	
	Marker* marker = pcalloc(marker);
	int markerIndex = MarkerManager_addMesh(w->mm, marker, "marker", 20); 
	
	{
		Vector2i loci;
		//printf("dynamic mesh spawn");
		loci.x = 30;
		loci.y = 30;
		
		// look up the height there.
		//getTerrainHeight(&w->map, &loci, 1, &h);
		float h = Map_getTerrainHeight(&w->map, loci);
		
		MarkerInstance inst;
		inst.pos = (Vector){loci.x, loci.y, h};
		inst.radius = 12;
		
		MarkerManager_addInstance(w->mm, markerIndex, &inst); 
	}
	// old bezier roads
	//w->roads = calloc(1, sizeof(*w->roads));
	//initRoadBlock(w->roads);
	
	//RoadControlPoint rcp = {
		//{5,5},
		//{50,50},
		//{40,7}
	//};
	//int id;
	//rbAddRoad(w->roads, &rcp, &id);
	
	//roadblock_update_vbo(w->roads);
	
	// water plane, temporary hack
	w->wp = calloc(1, sizeof(*w->wp));
	WaterPlane_create(w->wp, 200, &(Vector){0,0,0});
	
// 	Pipe_init(&w->testmesh);
	
	// hack to test lightmanager
	LightManager_AddPointLight(w->lm, (Vector){10,10, 10}, 200, 20);
	
	
	dynamicMeshManager_updateGeometry(w->dmm);
	MarkerManager_updateGeometry(w->mm);
	EmitterManager_updateGeometry(w->em);
	
	
	// very last thing: load textures
	TextureManager_loadAll(w->mapTexMan, (Vector2i){256, 256}); 
	TextureManager_loadAll(w->meshTexMan, (Vector2i){256, 256}); 
	TextureManager_loadAll(w->decalTexMan, (Vector2i){256, 256}); 
	TextureManager_loadAll(w->emitterTexMan, (Vector2i){256, 256}); 


	// terrain pass
	w->terrainPass = Map_CreateRenderPass(&w->map);
	w->terrainSelectionPass = Map_CreateSelectionPass(&w->map);
	
	// solids pass
	w->solidsPass = DynamicMeshManager_CreateRenderPass(w->dmm);

	// transparents pass
	w->transparentsPass = MarkerManager_CreateRenderPass(w->mm);
	
	// emitters TODO: put in the right pass
	w->emitterPass = EmitterManager_CreateRenderPass(w->em);
	
	// decals pass
	w->decalPass = DecalManager_CreateRenderPass(w->dm);
	RenderPass_addDrawable(w->decalPass, CustomDecalManager_CreateDrawable(w->cdm));
	
	
	// the sun shadow pass
	RenderPass* shadPass = DynamicMeshManager_CreateShadowPass(w->dmm);
	RenderPass* shadPass1 = Map_CreateShadowPass(&w->map);
	shadPass->clearDepth = 1;
	shadPass->drawBuffer = GL_NONE;
	shadPass->readBuffer = GL_NONE;
	ShadowMap_addPass(w->sunShadow, shadPass);
	ShadowMap_addPass(w->sunShadow, shadPass1);
	
	#include "../mods/World_initGL.generated_thunk.c" 
}



static Item* findItem(World* w, char* itemName) {
	int64_t index;
	
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
	
	inst->eid = newEID();
	inst->item = item;
	
	return inst;
}

static uint32_t spawnPart(World* w, ItemPart* part, uint32_t parentEID, Vector* center) {
	Vector loc;
	uint32_t eid = (1 << 31) - 2;
	
	vAdd(center, &part->offset, &loc);
	
	switch(part->type) {
		case ITEM_TYPE_DYNAMICMESH:
			eid = World_spawnAt_DynamicMesh(w, part->index, &loc);
			break;
		
		case ITEM_TYPE_STATICMESH:
			printf("!!! StaticMeshManager is obsolete. use DynamicMeshManager.\n");
			return 1 << 31;
		
		case ITEM_TYPE_EMITTER:
			eid = World_spawnAt_Emitter(w, part->index, &loc);
			break;

		case ITEM_TYPE_LIGHT:
			eid = World_spawnAt_Light(w, part->index, &loc);
			break;

		case ITEM_TYPE_DECAL:
			eid = World_spawnAt_Decal(w, part->index, &loc);
			break;
			
//		case ITEM_TYPE_CUSTOMDECAL: // TODO figure out spawning info
//			eid = World_spawnAt_CustomDecal(w, part->index, &loc);
// 			break;

		case ITEM_TYPE_MARKER:
			eid = World_spawnAt_Marker(w, part->index, &loc);
			break;
	
		default:
			printf("unknown part item type: %d, %d\n", part->type, part->index);
			return (1 << 31) - 1;
	}
	
	
	
	
	float h = Map_getTerrainHeight3f(&w->map, loc);
	
	
	C_RelativeInfo ri = {
		.parentEID = parentEID,
		.pos = (Vector){part->offset.x, part->offset.y, part->offset.z},
		.rotAxis = part->rotAxis,
		.rotTheta = part->rotTheta,
	};
	
	
	CES_addComponentName(&w->gs->ces, "relativeInfo", eid, &ri);
	
	
	return eid;
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
	
	return World_spawnAt_ItemPtr(w, item, location); 
}

int World_spawnAt_ItemPtr(World* w, Item* item, Vector* location) {
	int i;
	ItemInstance* inst;
	
	if(!item) return 1;
	
	inst = allocItemInstance(item);
	
	for(i = 0; i < item->numParts; i++) {
	//	printf("trying to spawn %d : %d, %s\n", item->parts[i].index, i, itemName);
		uint32_t peid = spawnPart(w, &item->parts[i], inst->eid, location);
		inst->parts[i].eid = peid;
		inst->parts[i].part = &item->parts[i];
		
		inst->parts[i].parentItemEID = inst->eid;
	}
	
	
	float h = Map_getTerrainHeight3f(&w->map, *location);
	
// 	float av = 1.0;
// 	CES_addComponentName(&w->gs->ces, "angularVelocity", inst->eid, &av);
	CES_addComponentName(&w->gs->ces, "position", inst->eid, &(Vector){location->x, location->y, location->z + h});
	C_Rotation rot = {{1.0, 0,0}, 2};
// 	CES_addComponentName(&w->gs->ces, "rotation", inst->eid, &rot);
	
	
	// insert into quadtree
	SceneItemInfo* si = QuadTree_allocSceneItem(&w->qt);
	si->aabb = (AABB2){
		{location->x - 1, location->y - 1},
		{location->x + 1, location->y + 1}
	};
	si->eid = inst->eid;
	QuadTree_insert(&w->qt, si);
	
	
	return inst->eid;
}


int World_spawnAt_DynamicMesh(World* w, int dmIndex, Vector* location) {
	DynamicMeshInstance dmi;
	float h;
	
	Vector2i loci;
	//printf("dynamic mesh spawn");
	loci.x = location->x;
	loci.y = location->y;
	
	// look up the height there.
	//getTerrainHeight(&w->map, &loci, 1, &h);
	h = Map_getTerrainHeight(&w->map, loci);
	
	//printf("map h at [%.1f, %.1f]: %.4f\n", location->x, location->y, h);
	
	// spawn instance
	dmi.pos.x = location->x;
	dmi.pos.y = location->y;
	dmi.pos.z = h + .5; // HACK +.5z for gazebo origin offset
	
	dmi.scale = 1;
	
	dmi.dir = (Vector){1, 0, 0};
	dmi.rot = 0;// F_PI / 5.0;
	
	dmi.alpha = 1.0;
	
	dynamicMeshManager_addInstance(w->dmm, dmIndex, &dmi);
	// note: dynamic mesh instances are updated every frame automatically
	
	uint16_t dmindex16 = dmIndex;
	uint32_t eid = newEID();
	CES_addComponentName(&w->gs->ces, "meshIndex", eid, &dmindex16);

	CES_addComponentName(&w->gs->ces, "position", eid, &dmi.pos);
	
	C_Rotation r = {
		{0, 0, 1},
		frand(0, 3.28)
	};
	CES_addComponentName(&w->gs->ces, "rotation", eid, &r);
	
	float av = 0;//frand(-2, 2);
	CES_addComponentName(&w->gs->ces, "angularVelocity", eid, &av);
	
	C_PathFollow pf = {
		.path = Path_makeRandomLoop(&dmi.pos, 50, 10, .01),
		.distTravelled = frand(0, 10000),
		.speed = frand(5, 100)
	};
//	CES_addComponentName(&w->gs->ces, "pathFollow", eid, &pf);
	
	return eid;
}


int World_spawnAt_Emitter(World* w, int emitterIndex, Vector* location) {
	float h;
	
	Vector2i loci;
	
	loci.x = location->x;
	loci.y = location->y;
	
	// look up the height there.
	//getTerrainHeight(&w->map, &loci, 1, &h);
	h = Map_getTerrainHeight(&w->map, loci);
	
	// printf("map h at [%.1f, %.1f]: %.4f\n", location->x, location->y, h);
	// emitter
	EmitterInstance inst = {
		.pos = (Vector){location->x, location->y, h + 2},
		.scale = 10,
		.start_time = 0,
		.lifespan = 1<<15
	};
	emitterIndex = 0;
	EmitterManager_addInstance(w->em, emitterIndex, &inst); 
	
	return newEID();
}

int World_spawnAt_Marker(World* w, int markerIndex, Vector* location) {
	MarkerInstance dmi;
	float h;
	
	Vector2i loci;
	//printf("dynamic mesh spawn");
	loci.x = location->x;
	loci.y = location->y;
	
	// look up the height there.
	//getTerrainHeight(&w->map, &loci, 1, &h);
	h = Map_getTerrainHeight(&w->map, loci);
	
	MarkerInstance inst;
	inst.pos = (Vector){location->x, location->y, h};
	inst.radius = 12;
	
	MarkerManager_addInstance(w->mm, markerIndex, &inst); 
	
	return newEID();
}

int World_spawnAt_Light(World* w, int lightIndex, Vector* location) {
	float h;
	
	Vector2i loci;
	Vector groundloc;
	
	loci.x = location->x;
	loci.y = location->y;
	// look up the height there.
	//getTerrainHeight(&w->map, &loci, 1, &h);
	h = Map_getTerrainHeight(&w->map, loci);
	
	groundloc = (Vector){location->x, location->y, h};

	//printf("spawning light %f,%f,%f\n", groundloc.x, groundloc.y, groundloc.z);

	
	LightManager_AddPointLight(w->lm, groundloc, 10.0, 0.02);
	
	return newEID();
}

int World_spawnAt_Decal(World* w, int index, Vector* location) {
	float h;
	
	Vector2i loci;
	Vector groundloc;
	
	loci.x = location->x;
	loci.y = location->y;
	// look up the height there.
	//getTerrainHeight(&w->map, &loci, 1, &h);
	h = Map_getTerrainHeight(&w->map, loci);
	
	groundloc = (Vector){location->x, location->y, h};

	//printf("spawning decal %f,%f,%f\n", groundloc.x, groundloc.y, groundloc.z);

	DecalInstance di;
	
	di.pos = groundloc;
	di.size = 3.0f;
	di.rot = 0.0f;
	di.alpha = 0.50f;
//	di.texIndex = 3;
//	di.tileInfo = 0;
	
	di.lerp1 = .15;
	
	DecalManager_AddInstance(w->dm, index, &di);
	
	return newEID();
}


int World_spawnAt_CustomDecal(World* w, int cdecalIndex, float width, const Vector2* p1, const Vector2* p2) {
	float h;
	int texIndex;
	
	Vector2i loci;
	Vector groundloc;
	CustomDecalInstance di;
	
	// look up the terrain heights
	//loci.x = p1->x;
	//loci.y = p1->y;	
	//getTerrainHeight(&w->map, &loci, 1, &h);
	//groundloc = (Vector){p1->x, p1->y, h};
	
	//loci.x = p2->x;
	//loci.y = p2->y;
	//getTerrainHeight(&w->map, &loci, 1, &h);
	//groundloc = (Vector){p2->x, p2->y, h};

	//printf("spawning custom decal %f,%f,%f\n", groundloc.x, groundloc.y, groundloc.z);

	Vector2 n;
	float hw = width / 2;
	
	Vector2 p12;
	vSub2(p2, p1, &p12);
	
	n = (Vector2){p12.y, -p12.x};
	vNorm2(&n, &n);
	vScale2(&n, hw, &n);
	
	vAdd(&n, p1, &di.pos1);
	vAdd(&n, p2, &di.pos3);
	
	vScale(&n, -1, &n);
	vAdd(&n, p1, &di.pos2);
	vAdd(&n, p2, &di.pos4);
	
	di.pos1.z = Map_getTerrainHeight3f(&w->map, di.pos1);
	di.pos2.z = Map_getTerrainHeight3f(&w->map, di.pos2);
	di.pos3.z = Map_getTerrainHeight3f(&w->map, di.pos3);
	di.pos4.z = Map_getTerrainHeight3f(&w->map, di.pos4);
	
	di.thickness = 50;
	
	di.tex12 = .5;
	di.tex34 = 3.0;
	//di.pos = groundloc;
	
// 	texIndex = VEC_ITEM(&w->cdm->decals, cdecalIndex)->texIndex; 
	
	CustomDecalManager_AddInstance(w->cdm, cdecalIndex, &di);
	
	return newEID();
}



void World_spawnAt_Road(World* w, Vector2* start,  Vector2* stop) {
	
	
	
	//printf("start/stop: %f,%f / %f,%f\n", start->x, start->y, stop->x, stop->y);
	
	RoadNode* i1 = RoadNetwork_GetNodeRadius(w->roads, start, 10);
	RoadNode* i2 = RoadNetwork_GetNodeRadius(w->roads, stop, 10);
	
	if(i1 == NULL) {
		i1 = RoadNetwork_AddNode(w->roads, *start);
	}
	
	if(i2 == NULL || i1 == i2) { // duplicate nodes can cause an fp exception deeper in the decal calculation
		i2 = RoadNetwork_AddNode(w->roads, *stop);
	}
		
	Road_AddEdge(w->roads, i1, i2);
	
	RoadNetwork_FlushDirty(w->roads, w);
	
}



void World_drawTerrain(World* w, PassFrameParams* pfp) {
	
	RenderPass_preFrameAll(w->terrainPass, pfp);
	RenderPass_renderAll(w->terrainPass, pfp->dp);
	RenderPass_postFrameAll(w->terrainPass);
}



void World_drawSolids(World* w, PassFrameParams* pfp) {
	//meshManager_draw(w->smm, view, proj);
	
	RenderPass_preFrameAll(w->solidsPass, pfp);
	RenderPass_renderAll(w->solidsPass, pfp->dp);
	RenderPass_postFrameAll(w->solidsPass);
	
	
	//glBlendFuncSeparatei(1, GL_SRC_COLOR, GL_ZERO, GL_SRC_ALPHA, GL_ZERO);
	//glBlendFuncSeparatei(1, GL_ONE, GL_ZERO, GL_SRC_ALPHA,  GL_ONE_MINUS_DST_COLOR);
	//glBlendFuncSeparatei(1, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparatei(1, GL_ONE, GL_ZERO, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	WaterPlane_draw(w->wp, pfp->dp->mWorldView, pfp->dp->mViewProj);
	
}


void World_preTransparents(World* w, PassFrameParams* pfp) {
	RenderPass_preFrameAll(w->transparentsPass, pfp);
}

void World_drawTransparents(World* w, PassFrameParams* pfp) {
	
	RenderPass_renderAll(w->transparentsPass, pfp->dp);
	
}

void World_postTransparents(World* w) {
	RenderPass_postFrameAll(w->transparentsPass);
}


void World_drawDecals(World* w, PassFrameParams* pfp) {
	
	// old bezier roads
	//drawRoad(w->roads, w->gs->depthTexBuffer, pfp->dp->mWorldView, pfp->dp->mViewProj);
	
	RenderPass_preFrameAll(w->decalPass, pfp);
	RenderPass_renderAll(w->decalPass, pfp->dp);
	RenderPass_postFrameAll(w->decalPass);
	
}



int World_lookUp_Item(World* w, char* name) {
	int64_t index;
	
	if(HT_get(&w->itemLookup, name, &index)) {
		fprintf(stderr, "!!! item not found: '%s'\n", name);
		return -1;
	}
	
	return index;
}

int World_lookUp_Part(World* w, enum ItemTypes type, char* name) {
	
}







/*


depth buf              w         -        w         r         -        -
      tex              -         r        -         -         r        r

light buf              w?        w?       w?        w?        w        -
      tex              -         -        -         -         -        r
       
         separate     full     depth    depth     depth      depth    GBuf
           FBOs       GBuf      tex     write     read       tex      tex
       |-----------|---------|--------|--------,----------|--------|---------|
         prepasses   terrain   decals   solids   emitters   lights   shading




shadows and reflections


*/














