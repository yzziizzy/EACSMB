#ifndef __EACSMB_dynamicMesh_h__
#define __EACSMB_dynamicMesh_h__




#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "texture.h"
#include "pcBuffer.h"


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
	
	float alpha, x1, x2, x3;
	// alpha, glow, blink
} DynamicMeshInstance;




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
	
	int texIndex; // index to meshman texture table
	
	VEC(DynamicMeshInstance) instances;
	VEC(Matrix) instMatrices;
	
} DynamicMesh;



typedef struct DynamicMeshManager {
	
	VEC(DynamicMesh*) meshes;
	HashTable(int) lookup;
	int totalVertices;
	int totalInstances;
	
	int maxInstances;
	
	//VEC(StaticMeshInstance*) instances;
	
	PCBuffer indirectCmds;
	PCBuffer instVB;
	
	// data for persistently mapped instance vbo
	GLuint instVBO;
	GLsync instFences[DMM_INST_VBO_BUFFER_DEPTH];
	int instNextRegion;
	size_t instRegionSize; // in bytes
	void* instDataPtr;
	
	GLuint geomVBO;
	GLuint geomIBO;
	
	VEC(Texture*) textures;
	HashTable(int) textureLookup;
	
} DynamicMeshManager;



void dynamicMeshManager_draw(DynamicMeshManager* mm, Matrix* view, Matrix* proj);
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

void dynamicMeshManager_updateMatrices(DynamicMeshManager* dmm);















#endif // __EACSMB_dynamicMesh_h__
