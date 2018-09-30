
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "component.h"

#include "utilities.h"





static inline void checkGrow(ComponentManager* cm, int increase);
static void grow(ComponentManager* cm);



uint32_t newEID() {
	static uint32_t nextEID = 1;
	return nextEID++;
}

void CES_init(CES* ces) {
	VEC_INIT(&ces->cms);
	HT_init(&ces->nameLookup, 4);
}

int CES_addComponentManager(CES* ces, ComponentManager* cm) {
	uint64_t index;
	
	if(HT_get(&ces->nameLookup, cm->name, &index)) {
		VEC_PUSH(&ces->cms, cm);
		HT_set(&ces->nameLookup, cm->name, VEC_LEN(&ces->cms) - 1); 
		return VEC_LEN(&ces->cms) - 1;
	}
	else {
		return VEC_ITEM(&ces->cms, index);
	}
}


int CES_addComponent(CES* ces, int compID, uint32_t eid, void* value) {
	ComponentManager* cm = VEC_ITEM(&ces->cms, compID);
	
	ComponentManager_add(cm, eid, value);
	
	return 0;
}


int CES_addComponentName(CES* ces, char* name, uint32_t eid, void* value) {
	uint64_t index;
	
	if(HT_get(&ces->nameLookup, name, &index)) {
		fprintf("component not found: %s\n", name);
		return -1;
	}
	
	return CES_addComponent(ces, index, eid, value);
}


ComponentManager* CES_getCompManager(CES* ces, char* name) {
	uint64_t index;
	
	if(HT_get(&ces->nameLookup, name, &index)) {
		fprintf("component not found: %s\n", name);
		return NULL;
	}
	
	return VEC_ITEM(&ces->cms, index);
}



// returns id
ComponentManager* ComponentManager_alloc(char* name, size_t compSize, int initialAlloc) {
	ComponentManager* cm;
	
	pcalloc(cm);
	ComponentManager_init(cm, name, compSize, initialAlloc);
	
	return cm;
}

int ComponentManager_init(ComponentManager* cm, char* name, size_t compSize, int initialAlloc) {
	
	cm->compElemSz = compSize;
	cm->compAlloc = compSize * initialAlloc;
	cm->compArray = calloc(1, cm->compAlloc);
	cm->compLen = 0;
	
	VEC_INIT(&cm->entIDs);
	
	cm->name = strdup(name);
	// TODO: look up name to get ID
	
	return cm->id;
}


void ComponentManager_add(ComponentManager* cm, uint32_t eid, void* value) {
	// leave internal spaces for additions sometimes?
	checkGrow(cm, 1);
	
	VEC_PUSH(&cm->entIDs, eid);
	memcpy(cm->compArray + cm->compLen * cm->compElemSz, value, cm->compElemSz);
	
	cm->compLen++;
}

void ComponentManager_compact() {
	// leave internal spaces for additions sometimes?
}



// returns null if not found
void* ComponentManager_get(ComponentManager* cm, uint32_t eid) {
	// binary search of entIDS
	
}




static inline void checkGrow(ComponentManager* cm, int increase) {
	if(cm->compLen + increase > cm->compAlloc / cm->compElemSz) grow(cm);
}


static void grow(ComponentManager* cm) {
	size_t newAlloc = cm->compAlloc * 2;
	void* p = realloc(cm->compArray, newAlloc);
	if(!p) {
		fprintf(stderr, "realloc failed in componentmanager for %d bytes\n", newAlloc);
		exit(1);
	}
	
	cm->compArray = p;
	cm->compAlloc = newAlloc;
	
	printf("growing component manager: %d \n", newAlloc);
}




