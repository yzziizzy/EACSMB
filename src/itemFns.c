


// empty callbacks


void part_spawn_UNKNOWN(void* partMgr, PartInstance* pi, void* partSpawnInfo) {}
void part_remove_UNKNOWN(void* partMgr, PartInstance* pi) {}
void part_move_UNKNOWN(void* partMgr, PartInstance* pi, Vector* newPos) {}

void part_spawn_ITEM(void* partMgr, PartInstance* pi, void* partSpawnInfo) {}
void part_remove_ITEM(void* partMgr, PartInstance* pi) {}
void part_move_ITEM(void* partMgr, PartInstance* pi, Vector* newPos) {}



// waiting to be moved to their appropriate files


void part_spawn_DYNAMICMESH(void* partMgr, PartInstance* pi, void* partSpawnInfo) {
	
}
void part_remove_DYNAMICMESH(void* partMgr, PartInstance* pi) {
	
}
void part_move_DYNAMICMESH(void* partMgr, PartInstance* pi, Vector* newPos) {
	
}


void part_spawn_EMITTER(void* partMgr, PartInstance* pi, void* partSpawnInfo) {
	
}
void part_remove_EMITTER(void* partMgr, PartInstance* pi) {
	
}
void part_move_EMITTER(void* partMgr, PartInstance* pi, Vector* newPos) {
	
}


void part_spawn_LIGHT(void* partMgr, PartInstance* pi, void* partSpawnInfo) {
	
}
void part_remove_LIGHT(void* partMgr, PartInstance* pi) {
	
}
void part_move_LIGHT(void* partMgr, PartInstance* pi, Vector* newPos) {
	
}


void part_spawn_DECAL(void* partMgr, PartInstance* pi, void* partSpawnInfo) {
	
}
void part_remove_DECAL(void* partMgr, PartInstance* pi) {
	
}
void part_move_DECAL(void* partMgr, PartInstance* pi, Vector* newPos) {
	
}



void part_spawn_MARKER(void* partMgr, PartInstance* pi, void* partSpawnInfo) {
	
}
void part_remove_MARKER(void* partMgr, PartInstance* pi) {
	
}
void part_move_MARKER(void* partMgr, PartInstance* pi, Vector* newPos) {
	
}


void part_spawn_SOUNDCLIP(void* partMgr, PartInstance* pi, void* partSpawnInfo) {
	
}
void part_remove_SOUNDCLIP(void* partMgr, PartInstance* pi) {
	
}
void part_move_SOUNDCLIP(void* partMgr, PartInstance* pi, Vector* newPos) {
	
}











// item spawning


void part_spawn_CUSTOMDECAL(void* partMgr, PartInstance* pi, void* info) {
	
	// this whole function needs to be fixed
	// these variables were undefined
	float width;
	int cdecalIndex;
	Vector* p1, *p2;
	
	
	// new vars
	World* w = (World*)partMgr;
	CustomDecalManager* cdm = w->cdm;
	
	
	
	
	float h;
	int texIndex;
	
	Vector2i loci;
	Vector groundloc;
	CustomDecalInstance* info_di = (CustomDecalInstance*)info;
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
	vSub2((Vector2*)p2, (Vector2*)p1, &p12);
	
	n = (Vector2){p12.y, -p12.x};
	vNorm2(&n, &n);
	vScale2(&n, hw, &n);
	
	vAdd2(&n, (Vector2*)p1, (Vector2*)&di.pos1);
	vAdd2(&n, (Vector2*)p2, (Vector2*)&di.pos3);
	
	vScale2(&n, -1, &n);
	vAdd2(&n, (Vector2*)p1, (Vector2*)&di.pos2);
	vAdd2(&n, (Vector2*)p2, (Vector2*)&di.pos4);
	
// 	di.pos1.z = Map_getTerrainHeight3f(&w->map, di.pos1);
// 	di.pos2.z = Map_getTerrainHeight3f(&w->map, di.pos2);
// 	di.pos3.z = Map_getTerrainHeight3f(&w->map, di.pos3);
// 	di.pos4.z = Map_getTerrainHeight3f(&w->map, di.pos4);
	
	di.thickness = 50;
	
	di.tex12 = .5;
	di.tex34 = 3.0;
	//di.pos = groundloc;
	
// 	texIndex = VEC_ITEM(&w->cdm->decals, cdecalIndex)->texIndex; 
	
// 	CustomDecalManager_AddInstance(w->cdm, cdecalIndex, &di);
	
	return newEID();
}

void part_remove_CUSTOMDECAL(void* partMgr, PartInstance* pi) {
	
}

void part_move_CUSTOMDECAL(void* partMgr, PartInstance* pi, Vector* newPos) {
	
}








