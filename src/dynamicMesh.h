#ifndef __EACSMB_dynamicMesh_h__
#define __EACSMB_dynamicMesh_h__




#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "settings.h"
#include "texture.h"
#include "pcBuffer.h"
#include "pass.h"
#include "mdi.h"

#include "gltf.h"

#include "component.h"



// must be at least 3. higher values may waste large amounts of vram
#define DMM_INST_VBO_BUFFER_DEPTH 3


typedef struct DynamicMeshVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
} DynamicMeshVertex;

typedef struct DynamicMeshInstance {
	Vector pos;
	float scale;
	
	Vector dir;
	float rot;
	
	// TODO: fix types
	float alpha;
	int diffuseIndex, normalIndex, metallicIndex, roughnessIndex;
	
} DynamicMeshInstance;


// this is what actually goes into the shader now
typedef struct DynamicMeshInstShader {
	Matrix m;
	
	// texture indices
	unsigned short diffuseIndex, normalIndex, metallicIndex, roughnessIndex;
} DynamicMeshInstShader;


typedef struct DynamicMesh {
	char* name;
	
	GLuint polyMode; // only GL_TRIANGLES supported, for now
	
	// always GL_TRIANGLES
	DynamicMeshVertex* vertices;
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
	VEC(DynamicMeshInstance) instances[2];
	VEC(Matrix) instMatrices;
	int numToDraw; // TODO: cycle per frame or move elsewhere
	
	size_t matBufOffset;
	
	float defaultScale;
	float defaultRotX;
	float defaultRotY;
	float defaultRotZ;
	
} DynamicMesh;



typedef struct DynamicMeshManager {
	
	VEC(DynamicMesh*) meshes;
	HashTable(int) lookup;
	int totalInstances;
	
	int maxInstances;
	
	Matrix* matBuf;
	size_t matBufAlloc;
	CES* ces; // hack for now to get the component managers
	
	TextureManager* tm;
	TextureManager* tmNorm;
	TextureManager* tmMat;
	MultiDrawIndirect* mdi;
	
	//VEC(Texture*) textures;
	//HashTable(int) textureLookup;
	
} DynamicMeshManager;



void dynamicMeshManager_draw(DynamicMeshManager* mm, PassFrameParams* pfp);
void dynamicMeshManager_updateGeometry(DynamicMeshManager* mm);
int dynamicMeshManager_addMesh(DynamicMeshManager* mm, char* name, DynamicMesh* sm);
int dynamicMeshManager_lookupName(DynamicMeshManager* mm, char* name);
int dynamicMeshManager_addInstance(DynamicMeshManager* mm, int meshIndex, const DynamicMeshInstance* smi);
int dynamicMeshManager_addTexture(DynamicMeshManager* mm, char* path);
DynamicMeshManager* dynamicMeshManager_alloc();
DynamicMesh* DynamicMeshFromOBJ(OBJContents* obj);
void DynamicMesh_updateBuffers(DynamicMesh* sm);
void drawDynamicMesh(DynamicMesh* m, Matrix* view, Matrix* proj);
void initDynamicMeshes();

DynamicMeshManager* dynamicMeshManager_alloc(GlobalSettings* gs);
void dynamicMeshManager_init(DynamicMeshManager* dmm, GlobalSettings* gs);
void dynamicMeshManager_initGL(DynamicMeshManager* dmm, GlobalSettings* gs); 

void dynamicMeshManager_updateMatrices(DynamicMeshManager* dmm, PassFrameParams* pfp);




RenderPass* DynamicMeshManager_CreateShadowPass(DynamicMeshManager* m);
RenderPass* DynamicMeshManager_CreateRenderPass(DynamicMeshManager* m);
PassDrawable* DynamicMeshManager_CreateDrawable(DynamicMeshManager* m);





// this is slow. don't use it for the main game.
typedef struct SlowMeshManager {
	VEC(DynamicMesh*) meshes;
	int maxMeshes;
	int maxInstances;
	int totalInstances;
	
	int totalVertices;
	int totalIndices;
	int indexSize;
	
	TextureManager* tm;
	TextureManager* tmNorm;
	TextureManager* tmMat;
	
	VAOConfig* vaoConfig;
	int vaoGeomStride;
	int vaoInstStride;
	GLuint vao;
	GLuint geomVBO;
	GLuint ibo;
	
	PCBuffer indirectCmds;
	PCBuffer instVB;
	
	char isIndexed;
	
} SlowMeshManager;


SlowMeshManager* SlowMeshManager_alloc(int maxInstances, int maxMeshes, VAOConfig* vaocfg);
void SlowMeshManager_init(SlowMeshManager* mm, int maxInstances, int maxMeshes, VAOConfig* vaocfg);
void SlowMeshManager_initGL(SlowMeshManager* mm);
void SlowMeshManager_RefreshGeometry(SlowMeshManager* mm);

RenderPass* SlowMeshManager_CreateShadowPass(SlowMeshManager* mm);
RenderPass* SlowMeshManager_CreateRenderPass(SlowMeshManager* mm);
PassDrawable* SlowMeshManager_CreateDrawable(SlowMeshManager* mm);





#endif // __EACSMB_dynamicMesh_h__
