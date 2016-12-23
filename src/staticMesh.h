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


typedef struct StaticMeshInstance {
	Vector pos;
	Vector dir;
	Vector scale;
	// alpha, glow, blink
} StaticMeshInstance;

struct buf_info {
	int alloc, cnt;
	int vertex_offset; // inda just crammed in here for now
};



typedef struct MeshManager {
	
	StaticMesh** meshes;
	int meshes_alloc;
	int meshes_cnt;
	int totalVertices;
	int totalInstances;
	
	StaticMeshInstance** instances;
	struct buf_info* inst_buf_info;
	
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
