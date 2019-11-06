#ifndef __EACSMB_builder_builder_h__
#define __EACSMB_builder_builder_h__


#include "../ds.h"
#include "../common_math.h"


#include "core.h"
#include "../dynamicMesh.h"



typedef struct MB_link {
	MB_operation* op;
	
	struct MB_link* parent;
	VEC(struct MB_link*) children;
	
	// dimensions, computed transformation, 
	Matrix compTrans; // total computed transformation
	
} MB_link;




typedef struct MeshBuilder {
	
	MeshData* md;
	DynamicMeshManager* mm;
	
	MB_operation* rootOp;
	MB_link* rootLink;
	MB_link* selectedLink;
	
	
} MeshBuilder;



enum {
	UP,
	DOWN,
	LEFT,
	RIGHT
};


MeshData* meshBuilder_test();
void MeshBuilder_LoadJSON(MeshBuilder* mb, char* path);
MeshBuilder* MeshBuilder_alloc();
void MeshBuilder_init(MeshBuilder* mb);


void MB_link_purgeChild(MB_link* p, MB_link* c); 
void MB_link_free(MB_link* l, int freeOp, int cascade);

#endif // __EACSMB_builder_builder_h__
