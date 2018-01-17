

#include "render.h" 



/*

	MeshData* md;
	MB_operation* root;
	
	
	root = read_json("assets/models/test.json");
	if(!root) {
		printf("failed to read test json file\n");
		exit(1);
	}
	
	md = process_op(root);
	if(!md) {
		printf("failed to process mesh operations \n");
		exit(1);
	}
*/




void updateMeshManager() {
	/*

	sm = calloc(1, sizeof(*sm));
	CHECK_OOM(sm);

	sm->vertexCnt = VEC_LEN(&md->verts);
	sm->indexCnt = VEC_LEN(&md->indices);
	sm->indexWidth = 2;
	
	printf("verts: %d, indices: %d \n", sm->vertexCnt, sm->indexCnt);
	
	
	sm->vertices = malloc(sizeof(*sm->vertices) * sm->vertexCnt);
	CHECK_OOM(sm->vertices);
	sm->indices.w16 = malloc(sizeof(*sm->indices.w16) * sm->indexCnt);
	CHECK_OOM(sm->indices.w16);

	for(i = 0; i < sm->vertexCnt; i++) sm->vertices[i] = *((StaticMeshVertex*)&VEC_ITEM(&md->verts, i));
	for(i = 0; i < sm->indexCnt; i++) sm->indices.w16[i] = VEC_ITEM(&md->indices, i);
	
	
	
	StaticMesh_updateBuffers(sm);
	*/
}













