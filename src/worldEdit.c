


#include "world.h"
#include "worldEdit.h"



void World_processEdit_Empty(World* w, EditCmd_Empty* e) {
	// intentionally left blank
}

void World_processEdit_Spawn(World* w, EditCmd_Spawn* e) {
	World_spawnAt_Item(w, e->name, &e->pos);
}
void World_processEdit_Move(World* w, EditCmd_Move* e) {
	
}



typedef void (*EditProcessingFn)(World*, EditCmd*);

EditProcessingFn editProcessors[] = {
	#define WORLD_EDIT_CMD(id, name) \
		[id] = (EditProcessingFn)World_processEdit_##name,
	WORLD_EDIT_CMD_LIST
	#undef WORLD_EDIT_CMD
};


void World_ProcessEdits(World* w, EditCmd* edits) {
	EditCmd* e = edits;
	
	while(1) {
		// loop control but also skip intentionally blanked records
		if(e->Empty.cmdID == EditCmd_Empty_ID) {
			if(e->Empty.is_terminator) break;
			continue;
		}
		
		EditProcessingFn fn = editProcessors[e->Empty.cmdID];
		if(!fn) {
			fprintf(stderr, "!!! Missing EditProcessingFn for %d\n", e->Empty.cmdID);
		}
		else {
			fn(w, e);
		}
		
		
		e++;
	}
} 
