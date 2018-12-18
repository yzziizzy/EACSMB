#ifndef __EACSMB__component_h__
#define __EACSMB__component_h__

#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "btree.h"

#include "c_json/json.h"



typedef union {
	int index;
	struct {
		int index;
		BPTNode* n;
	} bpt;
} CompManIter;



// component elements are always stored sorted by ent id ascending
// there may be gaps, where entid is zero
typedef struct ComponentManager {
	char* name;
	int id;
	
	int backend; 
	
	// vector-based version
	
	VEC(uint32_t) entIDs;
	
	// the VEC macros depend on a known size
	uint8_t* compArray;
	size_t compElemSz;
	size_t compAlloc;
	int compLen;
	
	
	// b+ tree-based version
	
	BPlusTree bptree;
	
	
	
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

void ComponentManager_loadConfig(CES* ces, json_value_t* root);



ComponentManager* ComponentManager_alloc(char* name, size_t compSize, int initialAlloc, int backend);
int ComponentManager_init(ComponentManager* cm, char* name, size_t compSize, int initialAlloc, int backend);

void ComponentManager_add(ComponentManager* cm, uint32_t eid, void* value);


void* ComponentManager_start(ComponentManager* cm, CompManIter* iter);
void* ComponentManager_nextEnt(ComponentManager* cm, CompManIter* iter, uint32_t matchingEnt);
void* ComponentManager_next(ComponentManager* cm, CompManIter* iter, uint32_t* eid);



uint32_t newEID();
	
	
	
	
	
	
// list of component structs, for now

typedef struct C_Rotation {
	Vector axis;
	float theta;
} C_Rotation;


#include "path.h"

typedef struct C_PathFollow {
	Path* path;
	float speed;
	float distTravelled;
} C_PathFollow;




struct RoadEdge;

typedef struct C_RoadWander {
	struct RoadEdge* edge;
	float speed;
	float distTravelled;
} C_RoadWander;



#endif // __EACSMB__component_h__
