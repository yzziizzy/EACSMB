#ifndef __EACSMB_bushes_h__
#define __EACSMB_bushes_h__


#include "common_math.h"
#include "ds.h"
#include "hash.h"

#include "texture.h"
#include "pass.h"
#include "mdi.h"


typedef struct BushConfig {
	int numPanes; // -1 to disable (default)
	int panesMin;
	int panesMax;
	
	float zVariation;
	
} BushConfig;



typedef struct BushInstance {
	Vector pos;
	float rot;
	
} BushInstance;

typedef struct BushModel {
	
	VEC(Vertex_PNT) vertices;
	VEC(unsigned short) indices;
	VEC(BushInstance) instances;
	
} BushModel;






typedef struct BushManager {
	
	VEC(BushModel*) meshes;
	
	HashTable(int) lookup;
	
	TextureManager* tm;
	MultiDrawIndirect* mdi;
	
	
	// wind textures
	
	// wind timers and spinners
	
	
} BushManager;


typedef struct BushInstanceShader {
	Vector pos; float rot;
	unsigned short diff, norm, met, rough;
} BushInstanceShader;




BushManager* BushManager_alloc(GlobalSettings* gs);
void BushManager_init(BushManager* bmm, GlobalSettings* gs);
void BushManager_initGL(BushManager* bmm, GlobalSettings* gs);
void BushManager_updateGeometry(BushManager* mm);


RenderPass* BushManager_CreateRenderPass(BushManager* bmm);
PassDrawable* BushManager_CreateDrawable(BushManager* bmm);


// returns the index of the mesh
int BushManager_addMesh(BushManager* bmm, BushModel* b, char* name);
void BushManager_addInstance(BushManager* bmm, int index, BushInstance* inst);

// temp
void bush_addQuad(BushModel* bm, Vector center, Vector2 size, float rotation, float tilt);


#endif // __EACSMB_bushes_h__
