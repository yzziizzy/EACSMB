#ifndef __EACSMB_STATICMESH_H__
#define __EACSMB_STATICMESH_H__


#include "ds.h"



typedef struct StaticMeshVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
} StaticMeshVertex;

typedef struct StaticMeshInstance {
	Vector pos;
	Vector dir;
	Vector scale;
	// alpha, glow, blink
} StaticMeshInstance;




typedef struct StaticMesh {
	
	// always GL_TRIANGLES
	StaticMeshVertex* vertices;
	int vertexCnt;
	
	GLuint vbo;
	GLuint texID;
	
	VEC(StaticMeshInstance) instances;
	
} StaticMesh;



typedef struct MeshManager {
	
	VEC(StaticMesh*) meshes;
	int totalVertices;
	int totalInstances;
	
	//VEC(StaticMeshInstance*) instances;
	
	int activePosVBO;
	// need a sync object
	GLuint instVBO;
	GLuint geomVBO;
	
} MeshManager;


void initStaticMeshes();
StaticMesh* StaticMeshFromOBJ(OBJContents* obj);

void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj);

MeshManager* meshManager_alloc();
void meshManager_draw(MeshManager* mm, Matrix* view, Matrix* proj);
int meshManager_addMesh(MeshManager* mm, StaticMesh* sm);
int meshManager_addInstance(MeshManager* mm, int meshIndex, const StaticMeshInstance* smi);
void meshManager_updateGeometry(MeshManager* mm);
void meshManager_updateInstances(MeshManager* mm);

#endif // __EACSMB_STATICMESH_H__
