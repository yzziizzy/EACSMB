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
	VEC(BushInstance*) instances;
	
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
	Matrix mat;
	unsigned short diff, norm, met, rough;
} BushInstanceShader;




BushManager* BushManager_alloc(GlobalSettings* gs);
void BushManager_init(BushManager* bmm, GlobalSettings* gs);
void BushManager_initGL(BushManager* bmm, GlobalSettings* gs);


RenderPass* BushManager_CreateRenderPass(BushManager* bmm);
PassDrawable* BushManager_CreateDrawable(BushManager* bmm);






#endif // __EACSMB_bushes_h__
