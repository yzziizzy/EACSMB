#ifndef __EACSMB_worldEdit_h__
#define __EACSMB_worldEdit_h__







struct World;
typedef struct World World;


// Never edit the ID
#define WORLD_EDIT_CMD_LIST \
	/*              ID  Name          */ \
	WORLD_EDIT_CMD(  0, Empty) \
	WORLD_EDIT_CMD(  1, Spawn) \
	WORLD_EDIT_CMD(  2, Move) \
	


#define WORLD_EDIT_CMD(id, name) \
	const static int EditCmd_##name##_ID = id;
WORLD_EDIT_CMD_LIST
#undef WORLD_EDIT_CMD

typedef struct EditCmd_Empty {
	uint32_t cmdID;
	
	uint32_t is_terminator;
} EditCmd_Empty;

// spawns relative to the ground level
typedef struct EditCmd_Spawn {
	uint32_t cmdID;
	
	char* name;
	
	Vector pos;
} EditCmd_Spawn;

typedef struct EditCmd_Move{
	uint32_t cmdID;
	
	uint32_t type;
	uint32_t id;
	
	Vector pos;
} EditCmd_Move;




typedef union EditCmd {
	#define WORLD_EDIT_CMD(id, name) EditCmd_##name name;
	WORLD_EDIT_CMD_LIST
	#undef WORLD_EDIT_CMD
} EditCmd;






#endif // __EACSMB_worldEdit_h__
