#ifndef __EACSMB_items_h__
#define __EACSMB_items_h__



struct World;
typedef struct World World;

#define PART_TYPE_LIST \
	PART_TYPE(UNKNOWN) \
	PART_TYPE(ITEM) \
	PART_TYPE(STATICMESH) \
	PART_TYPE(DYNAMICMESH) \
	PART_TYPE(EMITTER) \
	PART_TYPE(LIGHT) \
	PART_TYPE(DECAL) \
	PART_TYPE(CUSTOMDECAL) \
	PART_TYPE(MARKER) \
	PART_TYPE(SOUNDCLIP)




enum PartType {
#define PART_TYPE(name) PART_TYPE_##name,
	PART_TYPE_LIST
#undef PART_TYPE
};




enum PartFlag {
	PART_FLAG_SPAWN_IMMEDIATE, // spawn the part when spawning the item
	PART_FLAG_ATTACHED, // the part hsould be moved with the item
	
};



typedef struct { // information about a specific kind of part
	enum PartType type;
	int index;
	char* name;
} Part;


typedef struct { // info about how a certain part part relates to a certain item
	enum PartType type;
	int index;
	int partIndex;
	Vector offset; // rotation, scale, etc
	Vector rotAxis;
	float rotTheta;
	
	enum PartFlag flags;
	
	//void* data;
} ItemPart;






typedef struct Item {
	char* name;
	int numParts;
	ItemPart* parts;
	
} Item;



typedef struct PartInstance { // info about an instance of a certain part
	ItemPart* part;
// 	ItemInstance* item;
	
	void* localID;
	
	uint32_t parentItemEID;
	uint32_t eid;
} PartInstance;

typedef struct {
	Item* item;
	Vector pos;
	
	uint32_t eid;
	int numParts;
	
	PartInstance parts[];
} ItemInstance;



// callback signatures
typedef void (*item_move_fn)(void* /* partMgr */, PartInstance*, Vector* /* newPos */); 
typedef void (*item_remove_fn)(void*, PartInstance*); 
typedef void (*item_spawn_fn)(void*, PartInstance*, void* /* info */);

typedef struct ItemVTable {
	item_move_fn move;
	item_remove_fn remove;
	item_spawn_fn spawn;
} ItemVTable;






#endif // __EACSMB_items_h__
