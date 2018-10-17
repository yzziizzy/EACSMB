#ifndef __EACSMB_marker_h__
#define __EACSMB_marker_h__




#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "texture.h"
#include "pcBuffer.h"
#include "pass.h"
#include "mdi.h"

#include "component.h"






typedef struct MarkerVertex {
	Vector v;
	struct {
		unsigned short u, v;
	} t;
} MarkerVertex;



typedef struct MarkerInstance {
	Vector pos; 
	float radius;
	
	
} MarkerInstance;



typedef struct Marker {
	
	
} Marker;




typedef struct MarkerManager {
	
	VEC(Marker*) meshes;
	HashTable(int) lookup;
	int totalInstances;
	
	int maxInstances;
	
	CES* ces; // hack for now to get the component managers
	
	TextureManager* tm;
	MultiDrawIndirect* mdi;
	
	
} MarkerManager;


MarkerManager* MarkerManager_alloc(int maxInstances);
int MarkerManager_addMesh(MarkerManager* mm, char* name, int segments);
void MarkerManager_updateGeometry(MarkerManager* mm);

RenderPass* MarkerManager_CreateRenderPass(MarkerManager* m);
PassDrawable* MarkerManager_CreateDrawable(MarkerManager* m);





#endif // __EACSMB_marker_h__
