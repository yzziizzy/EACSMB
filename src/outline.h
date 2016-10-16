




// 2d version

struct outline_node {
	Vector start, end;
	
	// need to switch to line segments
}


struct outline {
	int len;
	int alloc;
	struct outline_node* lines;
	
	char is_loop;
	Vector extrusion_dir;
};





struct outline* outline_alloc(int node_cnt)
int outline_add_node(struct outline* o, struct outline_node* n)
int outline_add_node(struct outline* o, struct outline_node* n)














