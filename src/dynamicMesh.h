#ifndef __EACSMB_dynamicMesh_h__
#define __EACSMB_dynamicMesh_h__




#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "texture.h"
#include "pcBuffer.h"
#include "pass.h"


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
	float alpha, texIndex, x2, x3;
	
} DynamicMeshInstance;


// this is what actually goes into the shader now
typedef struct DynamicMeshInstShader {
	Matrix m;
	
	// texture indices
	unsigned short diffuseIndex, normalIndex;
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
	
	GLuint vbo;
	GLuint ibo;
	GLuint texID;
	
	int texIndex; // index to texture array
	
	int curFrameIndex;
	VEC(DynamicMeshInstance) instances[2];
	VEC(Matrix) instMatrices;
	int numToDraw; // TODO: cycle per frame or move elsewhere
	
	float defaultScale;
	float defaultRotX;
	float defaultRotY;
	float defaultRotZ;
	
} DynamicMesh;



typedef struct DynamicMeshManager {
	
	VEC(DynamicMesh*) meshes;
	HashTable(int) lookup;
	int totalVertices;
	int totalIndices;
	int totalInstances;
	
	int maxInstances;
	
	//VEC(StaticMeshInstance*) instances;
	
	PCBuffer indirectCmds;
	PCBuffer instVB;
	
	// data for persistently mapped instance vbo
	GLuint instVBO;
	GLuint geomVBO;
	GLuint geomIBO;
	
	TextureManager* tm;
	
	//VEC(Texture*) textures;
	//HashTable(int) textureLookup;
	
} DynamicMeshManager;



void dynamicMeshManager_draw(DynamicMeshManager* mm, PassFrameParams* pfp);
void dynamicMeshManager_updateGeometry(DynamicMeshManager* mm);
int dynamicMeshManager_addMesh(DynamicMeshManager* mm, char* name, DynamicMesh* sm);
int dynamicMeshManager_lookupName(DynamicMeshManager* mm, char* name);
int dynamicMeshManager_addInstance(DynamicMeshManager* mm, int meshIndex, const DynamicMeshInstance* smi);
int dynamicMeshManager_addTexture(DynamicMeshManager* mm, char* path);
void dynamicMeshManager_readConfigFile(DynamicMeshManager* mm, char* configPath);
DynamicMeshManager* dynamicMeshManager_alloc();
DynamicMesh* DynamicMeshFromOBJ(OBJContents* obj);
void DynamicMesh_updateBuffers(DynamicMesh* sm);
void drawDynamicMesh(DynamicMesh* m, Matrix* view, Matrix* proj);
void initDynamicMeshes();

DynamicMeshManager* dynamicMeshManager_alloc(int maxInstances);

void dynamicMeshManager_updateMatrices(DynamicMeshManager* dmm, PassFrameParams* pfp);















#endif // __EACSMB_dynamicMesh_h__
