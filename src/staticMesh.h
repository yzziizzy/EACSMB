#ifndef __EACSMB_STATICMESH_H__
#define __EACSMB_STATICMESH_H__





typedef struct StaticMeshVertex {
	Vector v, n;
	struct {
		unsigned short u, v;
	} t;
} StaticMeshVertex;


typedef struct StaticMesh {
	
	// always GL_TRIANGLES
	StaticMeshVertex* vertices;
	int vertexCnt;
	
	GLuint vbo;
	GLuint texID;
	
} StaticMesh;



typedef struct MeshManager {
	
	StaticMesh** meshes;
	int meshes_alloc;
	int meshes_cnt;
	
	int activePosVBO;
	GLuint instVBO[2];
	GLuint geomVBO;
	
} MeshManager;


void initStaticMeshes();
StaticMesh* StaticMeshFromOBJ(OBJContents* obj);

void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj);

MeshManager* meshManager_alloc();
void meshManager_draw(MeshManager* mm);
int meshManager_addMesh(MeshManager* mm, StaticMesh* sm);
void meshManager_updateGeometry(MeshManager* mm);
void meshManager_updateInstances(MeshManager* mm);

#endif // __EACSMB_STATICMESH_H__
