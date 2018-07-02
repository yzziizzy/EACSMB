#ifndef __EACSMB__component_h__
#define __EACSMB__component_h__

#include "common_math.h"

#include "ds.h"
#include "hash.h"


// component elements are always stored sorted by ent id ascending
// there may be gaps, where entid is zero


typedef struct ComponentManager {
	char* name;
	int id;
	
	VEC(uint32_t) entIDs;
	
	// the VEC macros depend on a known size
	uint8_t* compArray;
	size_t compElemSz;
	size_t compAlloc;
	int compLen;
	
	// TODO: queue for adding mid-frame
	
} ComponentManager;


typedef struct CES {
	VEC(ComponentManager*) cms;
	HashTable(ComponentManager*) nameLookup;
	
	
} CES;



void CES_init(CES* ces);
int CES_addComponentManager(CES* ces, ComponentManager* cm);
int CES_addComponent(CES* ces, int compID, uint32_t eid, void* value);
int CES_addComponentName(CES* ces, char* name, uint32_t eid, void* value);

ComponentManager* CES_getCompManager(CES* ces, char* name);


ComponentManager* ComponentManager_alloc(char* name, size_t compSize, int initialAlloc);
int ComponentManager_init(ComponentManager* cm, char* name, size_t compSize, int initialAlloc);

void ComponentManager_add(ComponentManager* cm, uint32_t eid, void* value);

// size_t index = -1;
// while(CM_next(cm, &index)) {  stuff  }
//
// set index to -1 to start
static inline void* ComponentManager_next(ComponentManager* cm, int* index, uint32_t* eid) {
	while(1) {
		(*index)++;
		if(*index >= cm->compLen) {  return NULL; }
		if((*eid = VEC_ITEM(&cm->entIDs, *index)) > 0) {
			return cm->compArray + cm->compElemSz * *index;
		}
	}
}



// used to find components matching an entity
static inline void* ComponentManager_nextEnt(ComponentManager* cm, int* index, uint32_t matchingEnt) {
	while(1) {
		uint32_t e;
		
		(*index)++;
		if(*index >= cm->compLen) return NULL;
		
		e = VEC_ITEM(&cm->entIDs, *index);
		
		if(e == matchingEnt) return cm->compArray + cm->compElemSz * *index;
		if(e > matchingEnt) return NULL;
	}
}




uint32_t newEID();
	
	
	
	
	
	
// list of component structs, for now

typedef struct C_Rotation {
	Vector axis;
	float theta;
} C_Rotation;



#endif // __EACSMB__component_h__
