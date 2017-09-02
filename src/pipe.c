


#include "pipe.h"


typedef struct MeshData {
	SMVList verts;
	VEC(int) indices;
	
} MeshData;

MeshData* mdcreate() {
	MeshData* md;
	
	md = calloc(1, sizeof(*md));
	CHECK_OOM(md);
	
	VEC_INIT(&md->verts);
	VEC_INIT(&md->indices);
	
	return md;
}




void initPipes() {
	
	
	
}



static void createTetrahedron(float size, MeshData* md) {
	StaticMeshVertex vert;
	
	
	vert = (StaticMeshVertex){
		.v = {0, 0, 0},
		.n = {.5, .5, 0},
		.t = {0, 0}
	};
	VEC_PUSH(&md->verts, vert);
	
	vert = (StaticMeshVertex){
		.v = {size, 0, 0},
		.n = {.5, .5, 0},
		.t = {0, 0}
	};
	VEC_PUSH(&md->verts, vert);
	
	vert = (StaticMeshVertex){
		.v = {0, size, 0},
		.n = {.5, .5, 0},
		.t = {0, 0}
	};
	VEC_PUSH(&md->verts, vert);
	
	vert = (StaticMeshVertex){
		.v = {0, 0, size},
		.n = {.5, .5, 0},
		.t = {0, 0}
	};
	VEC_PUSH(&md->verts, vert);
	
	// bottom triangle
	VEC_PUSH(&md->indices, 0);
	VEC_PUSH(&md->indices, 1);
	VEC_PUSH(&md->indices, 2);
	
	// hyp face
	VEC_PUSH(&md->indices, 1);
	VEC_PUSH(&md->indices, 2);
	VEC_PUSH(&md->indices, 3);

	// vert faces
	VEC_PUSH(&md->indices, 0);
	VEC_PUSH(&md->indices, 2);
	VEC_PUSH(&md->indices, 3);
	
	VEC_PUSH(&md->indices, 0);
	VEC_PUSH(&md->indices, 1);
	VEC_PUSH(&md->indices, 3);
	
}

static void createCylinder(float radius, float length, int sections, MeshData* md) {
	int i, j;
	StaticMeshVertex vert;
	
	
	// create vertices
	for(j = 0; j < 2; j++) {
		for(i = 0; i < sections; i++) {
			float theta = (i * D_2PI) / sections;
			
			vert.v.y = j * length;
			vert.v.x = cos(theta) * radius;
			vert.v.z = sin(theta) * radius;
			
			vert.n.y = 0;
			vert.n.x = cos(theta);
			vert.n.z = sin(theta);
			
			vert.t.u = j * (65535 / 1);//(j / length) * 65536; 
			vert.t.v = i * (65535 / sections); //(i / sections) * 65536; 
			
			VEC_PUSH(&md->verts, vert);
		}
	}
	
	// fill in indices
	int ring1 = 0, ring2 = sections;
	for(i = 0; i < sections; i++) {
		int r1v1 = ring1 + i;
		int r1v2 = ring1 + ((i + 1) % sections);
		int r2v1 = ring2 + i;
		int r2v2 = ring2 + ((i + 1) % sections);
		
		// triangle 1
		VEC_PUSH(&md->indices, r1v1);
		VEC_PUSH(&md->indices, r2v1);
		VEC_PUSH(&md->indices, r1v2);
		
		// triangle 2
		VEC_PUSH(&md->indices, r1v2);
		VEC_PUSH(&md->indices, r2v1);
		VEC_PUSH(&md->indices, r2v2);
	}
}





void Pipe_init(PipeSegment* ps) {
	MeshData*  md;
	StaticMesh* sm;
	int i;
	
	
	ps->length = 200;
	
	md = mdcreate();
	createCylinder(40, ps->length, 20, md);
	//createTetrahedron(10, md);
	
	// initialize a static mesh object and fill it with the mesh data
	sm = calloc(1, sizeof(*sm));
	CHECK_OOM(sm);

	sm->vertexCnt = VEC_LEN(&md->verts);
	sm->indexCnt = VEC_LEN(&md->indices);
	sm->indexWidth = 2;
	
	printf("verts: %d, indices: %d \n", sm->vertexCnt, sm->indexCnt);
	
	// alloc space in static mesh
	sm->vertices = malloc(sizeof(*sm->vertices) * sm->vertexCnt);
	CHECK_OOM(sm->vertices);
	sm->indices.w16 = malloc(sizeof(*sm->indices.w16) * sm->indexCnt);
	CHECK_OOM(sm->indices.w16);
	
	// copy data
	for(i = 0; i < sm->vertexCnt; i++) sm->vertices[i] = VEC_ITEM(&md->verts, i);
	for(i = 0; i < sm->indexCnt; i++) sm->indices.w16[i] = VEC_ITEM(&md->indices, i);
	
	for(i = 0; i < sm->vertexCnt; i++) {
// 		Vector* v = &sm->vertices[i].v;
// 		printf("%d - [%.2f, %.2f, %.2f]\n", i, v->x, v->y, v->z);
		uint16_t* t = &sm->vertices[i].t;
		printf("%d - [%d, %d]\n", i, t[0], t[1]);
	}

	for(i = 0; i < sm->indexCnt; i+=3) {
		short* v = &sm->indices.w16[i];
		printf("face %d - [%d, %d, %d]\n", i / 3, v[0], v[1], v[2]);
	}
	
	StaticMesh_updateBuffers(sm);
	
	ps->sm = sm;
	
	
	
}










