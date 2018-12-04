#ifndef __EACSMB_marker_h__
#define __EACSMB_marker_h__




#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "settings.h"
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



typedef struct MarkerInstanceShader {
	Vector pos; 
	float radius;
	
	unsigned short texIndex, divisor;
} MarkerInstanceShader;


typedef struct MarkerInstance {
	Vector pos; 
	float radius;
	
	double spawnTime;
	double deathTime;
} MarkerInstance;



typedef struct Marker {
	VEC(MarkerInstance) instances;
	
	int texIndex;
	char* texName;
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


MarkerManager* MarkerManager_alloc(GlobalSettings* gs);
void MarkerManager_init(MarkerManager* mm, GlobalSettings* gs);
void MarkerManager_initGL(MarkerManager* mm, GlobalSettings* gs);
int MarkerManager_addMesh(MarkerManager* mm, Marker* m, char* name, int segments);
void MarkerManager_addInstance(MarkerManager* mm, int index, MarkerInstance* inst);
void MarkerManager_updateGeometry(MarkerManager* mm);
int MarkerManager_lookupName(MarkerManager* mm, char* name);

RenderPass* MarkerManager_CreateRenderPass(MarkerManager* m);
PassDrawable* MarkerManager_CreateDrawable(MarkerManager* m);


void MarkerManager_readConfigFile(MarkerManager* mm, char* configPath); 


#endif // __EACSMB_marker_h__
