#ifndef __EACSMB_riggedMesh_h__
#define __EACSMB_riggedMesh_h__




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




// must be at least 3. higher values may waste large amounts of vram
#define RMM_INST_VBO_BUFFER_DEPTH 3


typedef struct RiggedMeshVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
	uint8_t boneIndices[4];
	float boneWeights[4];
} RiggedMeshVertex;

typedef struct RiggedMeshInstance {
	Vector pos;
	float scale;
	
	Vector dir;
	float rot;
	
	// TODO: fix types
	float alpha, texIndex, x2, x3;
	
} RiggedMeshInstance;


// this is what actually goes into the shader now
typedef struct RiggedMeshInstShader {
	Matrix m;
	
	// texture indices
	uint16_t diffuseIndex, normalIndex;
	
	uint32_t boneBuffeOffset; 
} RiggedMeshInstShader;


typedef struct RiggedMesh {
	char* name;
	
	GLuint polyMode; // only GL_TRIANGLES supported, for now
	
	// always GL_TRIANGLES
	RiggedMeshVertex* vertices;
	int vertexCnt;
	
	union {
		uint8_t* w8;
		uint16_t* w16;
		uint32_t* w32;
	} indices;
	int indexCnt;
	int indexWidth; // 0 = no index usage
		
	int texIndex; // index to texture array
	
	
	// rendering info
	int curFrameIndex;
	VEC(RiggedMeshInstance) instances[2];
	VEC(Matrix) instMatrices; // meshes for the model
	
	
	int numToDraw; // TODO: cycle per frame or move elsewhere
	
	size_t matBufOffset;
	
	float defaultScale;
	float defaultRotX;
	float defaultRotY;
	float defaultRotZ;
	
} RiggedMesh;



typedef struct RiggedMeshManager {
	
	VEC(RiggedMesh*) meshes;
	HashTable(int) lookup;
	int totalInstances;
	
	int maxInstances;
	
	Matrix* matBuf;
	size_t matBufAlloc;
	CES* ces; // hack for now to get the component managers
	
	TextureManager* tm;
	MultiDrawIndirect* mdi;
	PCBuffer bonesBuffer;
	
	//VEC(Texture*) textures;
	//HashTable(int) textureLookup;
	
} RiggedMeshManager;



void RiggedMeshManager_draw(RiggedMeshManager* mm, PassFrameParams* pfp);
void RiggedMeshManager_updateGeometry(RiggedMeshManager* mm);
int RiggedMeshManager_addMesh(RiggedMeshManager* mm, char* name, RiggedMesh* sm);
int RiggedMeshManager_lookupName(RiggedMeshManager* mm, char* name);
int RiggedMeshManager_addInstance(RiggedMeshManager* mm, int meshIndex, const RiggedMeshInstance* smi);
int RiggedMeshManager_addTexture(RiggedMeshManager* mm, char* path);
RiggedMeshManager* RiggedMeshManager_alloc();
RiggedMesh* RiggedMeshFromOBJ(OBJContents* obj);
void RiggedMesh_updateBuffers(RiggedMesh* sm);
void drawRiggedMesh(RiggedMesh* m, Matrix* view, Matrix* proj);
void initRiggedMeshes();

RiggedMeshManager* RiggedMeshManager_alloc(GlobalSettings* gs);
void RiggedMeshManager_init(RiggedMeshManager* dmm, GlobalSettings* gs);
void RiggedMeshManager_initGL(RiggedMeshManager* dmm, GlobalSettings* gs); 

void RiggedMeshManager_updateMatrices(RiggedMeshManager* dmm, PassFrameParams* pfp);




RenderPass* RiggedMeshManager_CreateShadowPass(RiggedMeshManager* m);
RenderPass* RiggedMeshManager_CreateRenderPass(RiggedMeshManager* m);
PassDrawable* RiggedMeshManager_CreateDrawable(RiggedMeshManager* m);






#endif // __EACSMB_riggedMesh_h__
