
















void MeshBuilder_Init(MeshBuilder* mb) {
	
	
	
	
}



// rebuild the mesh from the root operation
void MeshBuilder_Rebuild(MeshBuilder* mb) {
	
	
	
	mb->md = process_op(mb->rootOp);
	if(!md) {
		printf("failed to process mesh operations \n");
		exit(1);
	}
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
	
	if(freeOp) MB_operation_free(l->op);
	
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






