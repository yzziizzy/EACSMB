#ifndef __EACSMB_decals_custom_h__
#define __EACSMB_decals_custom_h__


#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "mempool.h"

#include "settings.h"
#include "texture.h"
#include "pcBuffer.h"
#include "pass.h"





typedef struct CustomDecalVertex {
	Vector v;
	struct {
		unsigned short u, v;
	} t;
} __attribute__ ((packed))  CustomDecalVertex;


typedef struct CustomDecalInstance {
	Vector pos1; // shouldn't this only be 2D?
	float thickness;
	
	Vector pos2;
	float alpha;
	
	Vector pos3;
	float tex12;
	
	Vector pos4;
	float tex34;
	
	unsigned short texIndex;
	unsigned short tileInfo; // 0 = clamp to transparent, 1 = tilex, 2 = tiley, 3 = tile both
	
}  __attribute__ ((packed)) CustomDecalInstance;


// mostly used for render order
typedef struct CustomDecal {
	char* name;
	
	int texIndex;
	
	float renderWeight;
	
	float thickness;
	VECMP(CustomDecalInstance) instances;
// 	VEC(CustomDecalInstance*) ephInstances;
	int numToDraw; // todo: move
} CustomDecal;


typedef struct CustomDecalManager {
	
	VEC(CustomDecal*) decals;
	HashTable(int) lookup;
	VEC(int) renderOrder; // indices into the decals vector

	int totalInstances;
	int maxInstances;

	PCBuffer indirectCmds;
	PCBuffer instVB;
	
	
	// data for persistently mapped instance vbo
	GLuint dtex;
	GLuint instVBO;
	//GLuint geomVBO;
	//GLuint geomIBO;
	
	TextureManager* tm;

// 	MemPool* instPool;

} CustomDecalManager;


PassDrawable* CustomDecalManager_CreateDrawable(CustomDecalManager* lm);
RenderPass* CustomDecalManager_CreateRenderPass(CustomDecalManager* lm);

int CustomDecalManager_AddDecal(CustomDecalManager* dm, char* name, CustomDecal* d);
void CustomDecalManager_updateMatrices(CustomDecalManager* dm, PassFrameParams* pfp);
int CustomDecalManager_lookupName(CustomDecalManager* dm, char* name);
CustomDecalInstance* CustomDecalManager_AddInstance(CustomDecalManager* dm, int index, const CustomDecalInstance* di);
void CustomDecalManager_DelInstance(CustomDecalManager* dm, CustomDecalInstance* di);

// CustomDecalInstance* CustomDecalManager_AddEphInstance(CustomDecalManager* dm, int index, const CustomDecalInstance* di);
// void CustomDecalManager_DelEphInstance(CustomDecalManager* dm, CustomDecalInstance* di);

CustomDecalManager* CustomDecalManager_alloc(GlobalSettings* gs); 
void CustomDecalManager_init(CustomDecalManager* dm, GlobalSettings* gs); 
void CustomDecalManager_initGL(CustomDecalManager* dm, GlobalSettings* gs); 
void initCustomDecals(); 



#endif // __EACSMB_decals_custom_h__
