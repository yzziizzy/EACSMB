
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


int CES_delComponent(CES* ces, int compID, uint32_t eid) {
	ComponentManager* cm = VEC_ITEM(&ces->cms, compID);
	
	ComponentManager_del(cm, eid);
	
	return 0;
}

int CES_delComponentName(CES* ces, char* name, uint32_t eid) {
	uint64_t index;
	
	if(HT_get(&ces->nameLookup, name, &index)) {
		fprintf("component not found: %s\n", name);
		return -1;
	}
	
	return CES_delComponent(ces, index, eid);
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
ComponentManager* ComponentManager_alloc(char* name, size_t compSize, int initialAlloc, int backend) {
	ComponentManager* cm;
	
	pcalloc(cm);
	ComponentManager_init(cm, name, compSize, initialAlloc, backend);
	
	return cm;
}

int ComponentManager_init(ComponentManager* cm, char* name, size_t compSize, int initialAlloc, int backend) {
	cm->backend = backend;
	
	if(backend == 0) { // vector
		cm->compElemSz = compSize;
		cm->compAlloc = compSize * initialAlloc;
		cm->compArray = calloc(1, cm->compAlloc);
		cm->compLen = 0;
		
		VEC_INIT(&cm->entIDs);
	}
	else if(backend == 1) { // b+ tree
		cm->bptree.keySz = sizeof(uint32_t);
		cm->bptree.valSz = compSize;
		
		int N = 4;
		int L = 16;
		if(N < 32 || L < 2048) 
			printf("WARNING: b+ tree node values in debug mode: N=%d L=%d\n", N, L);
		
		bpt_init(&cm->bptree, N, L); // these numbers should be more like 32, 4096
	}
	else if(backend == 2) { // MemPoolT
		
	}
	
	cm->name = strdup(name);
	// TODO: look up name to get ID
	
	
	return cm->id;
}


void ComponentManager_add(ComponentManager* cm, uint32_t eid, void* value) {
	
	if(cm->backend == 0) {
		// leave internal spaces for additions sometimes?
		checkGrow(cm, 1);
		
		VEC_PUSH(&cm->entIDs, eid);
		memcpy(cm->compArray + cm->compLen * cm->compElemSz, value, cm->compElemSz);
		
		cm->compLen++;
	}
	else if(cm->backend == 1) {
		bpt_insert(&cm->bptree, eid, value);
	}
}


void ComponentManager_del(ComponentManager* cm, uint32_t eid) {
	if(cm->backend == 0) {
		printf("Component delete operation is not supported on old backend\n");
	}
	else if(cm->backend == 1) {
		bpt_delete(&cm->bptree, eid);
	}
}



// returns null if not found
void* ComponentManager_find(ComponentManager* cm, uint32_t eid) {
	void* val;
	
	if(bpt_find(&cm->bptree, eid, &val)) {
		return val;
	}
	
	return NULL;
}



// size_t index = -1;
// while(CM_next(cm, &index)) {  stuff  }
//
// set index to -1 to start
void* ComponentManager_start(ComponentManager* cm, CompManIter* iter) {
	if(cm->backend == 0) { // vector
		iter->index = -1;
	}
	else if(cm->backend == 1) { // vector
		iter->bpt.index = -1;
	}
}

void* ComponentManager_next(ComponentManager* cm, CompManIter* iter, uint32_t* eid) {
	
	if(cm->backend == 0) { // vector
		while(1) {
			iter->index++;
			if(iter->index >= cm->compLen) {  return NULL; }
			if((*eid = VEC_ITEM(&cm->entIDs, iter->index)) > 0) {
				return cm->compArray + cm->compElemSz * iter->index;
			}
		}
	}
	else if(cm->backend == 1) { // b+ tree
		void* val;
		int ret;
		
		if(iter->bpt.index == -1) {
			ret = bpt_first(&cm->bptree, &iter->bpt.n, &iter->bpt.index, eid, &val);
		}
		else {
			ret = bpt_next(&cm->bptree, &iter->bpt.n, &iter->bpt.index, eid, &val);
		}
		
		return ret ? val : NULL;
	}
}



// used to find components matching an entity
void* ComponentManager_nextEnt(ComponentManager* cm, CompManIter* iter, uint32_t matchingEnt) {
	
	if(cm->backend == 0) { // vector
		while(1) {
			uint32_t e;
			
			iter->index++;
			if(iter->index >= cm->compLen) return NULL;
			
			e = VEC_ITEM(&cm->entIDs, iter->index);
			
			if(e == matchingEnt) return cm->compArray + (cm->compElemSz * iter->index);
			if(e > matchingEnt) return NULL;
		}
	}
	else if(cm->backend == 1) { // b+ tree
		void* val;
		int ret;
		uint32_t eid;
		
		
// 		if(iter->bpt.index == -1) {
// 			ret = bpt_first(&cm->bptree, &iter->bpt.n, &iter->bpt.index, &eid, &val);
// 		}
	//	printf("eid in nextEnt: %d\n", matchingEnt);
		ret = bpt_seek(&cm->bptree, &iter->bpt.n, &iter->bpt.index, matchingEnt, &eid, &val);
		
		
		
		return ret ? val : NULL;
	}
}




static int typeSizeLookup(char* type) {
	if(0 == strcmp("float", type)) {
		return sizeof(float);
	}
	else if(0 == strcmp("int", type)) {
		return 4;
	}
	else if(0 == strcmp("short", type)) {
		return 2;
	}
	else if(0 == strcmp("byte", type)) {
		return 1;
	}
	else if(0 == strcmp("char", type)) {
		return 1;
	}
	else if(0 == strcmp("long", type)) {
		return 8;
	}
	else if(0 == strcmp("double", type)) {
		return 8;
	}
	else if(0 == strcmp("pointer", type)) {
		return sizeof(void*);
	}

	else {
		printf("Unknown component type: %s\n", type);
		return -1;
	}
}




void ComponentManager_loadConfig(CES* ces, json_value_t* root) {
	
	json_value_t* jcomps;
	struct json_array_node* link;
	
	json_obj_get_key(root, "components", &jcomps);
	
	
	if(jcomps->type != JSON_TYPE_ARRAY) {
		printf("component config list not found\n");
		return;
	}
	
	
	link = jcomps->v.arr->head;
	while(link) {
		json_value_t* j_comp;
		json_value_t* v;
		char* name;
		int typeSize;
		
		ComponentManager* cm;
		
		
		if(link->value->type != JSON_TYPE_OBJ) {
			printf("invalid component format\n");
			goto CONTINUE;
		}
		j_comp = link->value;
		
		name = json_obj_get_string(j_comp, "name");
		typeSize = typeSizeLookup(json_obj_get_string(j_comp, "type"));
		if(typeSize < 0) {
			printf("invalid component size\n");
			goto CONTINUE;
		}
		
		printf("adding component %s with size %d\n", name, typeSize);
		cm = ComponentManager_alloc(name, typeSize, 1024*8, 1);
		
		CES_addComponentManager(ces, cm);
		
		
		
	CONTINUE:
		link = link->next;
	}
	
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




