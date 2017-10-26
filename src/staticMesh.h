#ifndef __EACSMB_STATICMESH_H__
#define __EACSMB_STATICMESH_H__

#include "common_gl.h"
#include "common_math.h"

#include "ds.h"
#include "hash.h"

#include "pcBuffer.h"
#include "texture.h"


typedef struct StaticMeshVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
} StaticMeshVertex;

typedef struct StaticMeshInstance {
	Vector pos;
	float scale;
	
	Vector dir;
	float rot;
	
	float alpha, x1, x2, x3;
	// alpha, glow, blink
} StaticMeshInstance;




typedef struct StaticMesh {
	char* name;
	
	GLuint polyMode; // only GL_TRIANGLES supported, for now
	
	// always GL_TRIANGLES
	StaticMeshVertex* vertices;
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
	
	VEC(StaticMeshInstance) instances;
	
} StaticMesh;



typedef struct MeshManager {
	
	VEC(StaticMesh*) meshes;
	HashTable(int) lookup;
	int totalVertices;
	int totalIndices;
	int totalInstances;
	
	//VEC(StaticMeshInstance*) instances;
	
	PCBuffer indirectCmds;
	
	int activePosVBO;
	// need a sync object
	GLuint instVBO;
	GLuint geomVBO;
	GLuint geomIBO;
	
	VEC(Texture*) textures;
	HashTable(int) textureLookup;
	
} MeshManager;


void initStaticMeshes();
StaticMesh* StaticMeshFromOBJ(OBJContents* obj);

void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj);
void StaticMesh_updateBuffers(StaticMesh* sm);

MeshManager* meshManager_alloc();
void meshManager_draw(MeshManager* mm, Matrix* view, Matrix* proj);
int meshManager_addMesh(MeshManager* mm, char* name, StaticMesh* sm);
int meshManager_addInstance(MeshManager* mm, int meshIndex, const StaticMeshInstance* smi);
void meshManager_updateGeometry(MeshManager* mm);
void meshManager_updateInstances(MeshManager* mm);

#endif // __EACSMB_STATICMESH_H__
