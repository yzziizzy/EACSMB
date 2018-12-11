#ifndef __EACSMB_decals_h__
#define __EACSMB_decals_h__


#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "settings.h"
#include "texture.h"
#include "pcBuffer.h"
#include "pass.h"





typedef struct DecalVertex {
	Vector v;
	struct {
		unsigned short u, v;
	} t;
} __attribute__ ((packed))  DecalVertex;


typedef struct DecalInstance {
	Vector pos; // shouldn't this only be 2 components?
	float size;
	
	float rot;
	float alpha;
	float lerp1, unused;
	
	unsigned short texIndex;
	unsigned short tileInfo; // 0 = clamp to transparent, 1 = tilex, 2 = tiley, 3 = tile both
	
}  __attribute__ ((packed)) DecalInstance;

typedef struct Decal {
	char* name;
	
	int texIndex;
	
	float renderWeight;
	
	float size;
	VEC(DecalInstance) instances;
	int numToDraw; // todo: move
} Decal;


typedef struct DecalManager {
	
	VEC(Decal*) decals;
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


} DecalManager;


PassDrawable* DecalManager_CreateDrawable(DecalManager* lm);
RenderPass* DecalManager_CreateRenderPass(DecalManager* lm);

int DecalManager_AddDecal(DecalManager* dm, char* name, Decal* d);
void DecalManager_updateMatrices(DecalManager* dm, PassFrameParams* pfp);
int DecalManager_lookupName(DecalManager* dm, char* name);
int DecalManager_AddInstance(DecalManager* dm, int index, const DecalInstance* di);
DecalManager* DecalManager_alloc(GlobalSettings* gs); 
void DecalManager_init(DecalManager* dm, GlobalSettings* gs); 
void DecalManager_initGL(DecalManager* dm, GlobalSettings* gs);

void initDecals(); 



#endif // __EACSMB_decals_h__
