

#include "builder.h"














MeshBuilder* MeshBuilder_alloc() {
	MeshBuilder* mb;
	mb = calloc(1, sizeof(*mb));
	
	MeshBuilder_init(mb);

	return mb;
}


void MeshBuilder_init(MeshBuilder* mb) {
	
	
	
	
}



// rebuild the mesh from the root operation
void MeshBuilder_Rebuild(MeshBuilder* mb) {
	
	StaticMesh* sm;
	int i;
	
	
	mb->md = process_op(mb->rootOp);
	if(!mb->md) {
		printf("failed to process mesh operations \n");
		exit(1);
	}
	
	
	sm = calloc(1, sizeof(*sm));
	CHECK_OOM(sm);

	sm->vertexCnt = VEC_LEN(&mb->md->verts);
	sm->indexCnt = VEC_LEN(&mb->md->indices);
	sm->indexWidth = 2;
	
	printf("verts: %d, indices: %d \n", sm->vertexCnt, sm->indexCnt);
	
	// alloc space in static mesh
	sm->vertices = malloc(sizeof(*sm->vertices) * sm->vertexCnt);
	CHECK_OOM(sm->vertices);
	sm->indices.w16 = malloc(sizeof(*sm->indices.w16) * sm->indexCnt);
	CHECK_OOM(sm->indices.w16);
	
	// copy data
	for(i = 0; i < sm->vertexCnt; i++) sm->vertices[i] = *((StaticMeshVertex*)&VEC_ITEM(&mb->md->verts, i));
	for(i = 0; i < sm->indexCnt; i++) sm->indices.w16[i] = VEC_ITEM(&mb->md->indices, i);
	
	
	//StaticMesh_updateBuffers(sm); 
	
	// TODO: purge mesh manager of all existing instances and meshes
	
	int m_index = meshManager_addMesh(mb->mm, "object", sm);
	
	StaticMeshInstance smi = {
		.pos = {0,0,0},
		.scale = 1.0,
		.dir = {0,1,0},
		.rot = 0,
		.alpha = .5
	};
	meshManager_addInstance(mb->mm, m_index, &smi);
	meshManager_updateGeometry(mb->mm);
	meshManager_updateInstances(mb->mm);
}


void MeshBuilder_LoadJSON(MeshBuilder* mb, char* path) {
	
	mb->rootOp = mb_op_read_json(path);
	if(!mb->rootOp) {
		printf("failed to read test json file '%s'\n", path);
		exit(1);
	}
	
	MeshBuilder_Rebuild(mb);
}


void MeshBuilder_MoveSelection(MeshBuilder* mb, int direction) {
	int i;
	int d = 1;
	
	switch(direction) {
		case UP:
			if(!mb->selectedLink) return;
			mb->selectedLink = mb->selectedLink->parent;
			return;
			
		case DOWN:
			if(!mb->selectedLink) {
				mb->selectedLink = mb->rootLink;
				return;
			}
			
			if(VEC_LEN(&mb->selectedLink->children)) {
				mb->selectedLink = VEC_ITEM(&mb->selectedLink->children, 0);
			}
			return;
		
		case LEFT:
			d = -1;
		case RIGHT:
			if(!mb->selectedLink) return;
			if(!mb->selectedLink->parent) return;
			
			i = VEC_FIND(&mb->selectedLink->parent->children, mb->selectedLink);
			
			mb->selectedLink = VEC_ITEM(
				&mb->selectedLink->parent->children,
				(i + d) % VEC_LEN(&mb->selectedLink->parent->children)
			);
			return;
		
		default:
			fprintf(stderr, "Invalid direction: %d (MeshBuilder_MoveSelection)\n", direction);
	}
}







MB_link* MB_link_alloc() {
	MB_link* l;
	
	l = calloc(1, sizeof(*l));
	CHECK_OOM(l);
	
	return l;
}



void MB_link_free(MB_link* l, int freeOp, int cascade) {
	if(!l) return;
	
	if(freeOp) MB_free(l->op);
	
	if(cascade) {
		int i;
		for(i = 0; i < VEC_LEN(&l->children); i++) {
			MB_link_free(VEC_ITEM(&l->children, i), freeOp, 1);
		}
	}
	
	// remove from the parent
	MB_link_purgeChild(l->parent, l);
}

void MB_link_purgeChild(MB_link* p, MB_link* c) {
	int i;
	MB_link* l;
	
	for(i = 0; i < VEC_LEN(&l->children); i++) {
		l = VEC_ITEM(&l->children, i);
		if(l == c) {
			VEC_RM_SAFE(&l->children, i);
		}
	}
}






