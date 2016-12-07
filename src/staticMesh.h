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
	
	StaticMesh* meshes;
	int meshes_alloc;
	int meshes_cnt;
	
	int activePosVBO;
	GLuint posVBO[2];
	GLuint geomVBO;
	
} MeshManager;


void initStaticMeshes();
StaticMesh* StaticMeshFromOBJ(OBJContents* obj);

void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj);

void meshManager_alloc(MeshManager* mm);

#endif // __EACSMB_STATICMESH_H__
