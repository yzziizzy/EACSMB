

#include "btree.h"



#define KEY_ARR(node) ((bpt_key_t*)(&((node)->d + 0)))
#define DATA_ARR(node, tree) ((bpt_val_t*)(&((node)->d + (tree->dataSz * tree->numPairs))))

#define KEY(tree, node, index) ((bpt_key_t)(*(KEY_ARR(node) + (((tree)->keySz) * (index)))))
#define DATA(tree, node, index) ((bpt_val_t)(*(DATA_ARR(node) + (((tree)->dataSz) * (index)))))




static inline int bpt_is_leaf(BPTNode* n) {
	return !n->type;
}
static inline int bpt_is_internal(BPTNode* n) {
	return !bpt_is_leaf(n);
}

static inline void* bpt_get_pairs(BPlusTree* tree, BPTNode* n) {
	return &n->d[0];
}

static inline int bpt_is_full(BPlusTree* tree, BPTNode* n) {
	if(bpt_is_leaf(n)) {
		return n->fill < tree->L + 1;
	}

	return n->fill < tree->N + 1;
}

//////////////////
static inline void* bpt_get_node_index(BPlusTree* tree, BPTNode* n, int index) {
	void* p = bpt_get_pairs(tree, n);
	return p + ((tree->keySz + sizeof(BPTNode*)) * index);
}

static inline bpt_key_t bpt_get_node_key(BPlusTree* tree, BPTNode* n, int index) {
	return *((bpt_key_t*)(bpt_get_node_index(tree, n, index) + 0));
}

static inline BPTNode* bpt_get_node_ptr(BPlusTree* tree, BPTNode* n, int index) {
	return (BPTNode*)(*(bpt_get_node_index(tree, n, index) + tree->keySz));
}

static inline void* bpt_seek_node_index(BPlusTree* tree, BPTNode* n, void* p, int seek) {
	return p + ((tree->keySz + sizeof(BPTNode*)) * seek);
}


////////////////////
static inline void* bpt_get_leaf_index(BPlusTree* tree, BPTNode* n, int index) {
	void* p = bpt_get_pairs(tree, n);
	return p + ((tree->pairSz) * index);
}

static inline bpt_key_t bpt_get_leaf_key(BPlusTree* tree, BPTNode* n, int index) {
	return *((bpt_key_t*)(bpt_get_leaf_index(tree, n, index) + 0));
}

static inline void* bpt_get_leaf_val(BPlusTree* tree, BPTNode* n, int index) {
	return bpt_get_leaf_index(tree, n, index) + tree->keySz;
}

static inline void* bpt_seek_leaf_index(BPlusTree* tree, BPTNode* n, void* p, int seek) {
	return p + ((tree->pairSz) * seek);
}

static inline BPTNode** bpt_get_leaf_ptr(BPlusTree* tree, BPTNode* n) {
	return (BPTNode**)bpt_get_leaf_index(tree, n, n->fill);
}



////////// allocation ////////////
static inline size_t bpt_node_data_size(BPlusTree* tree) {
	return (tree->N * tree->keySz) + ((tree->N + 1) * sizeof(BPTNode*));
}
static inline size_t bpt_leaf_data_size(BPlusTree* tree) {
	return (tree->L * tree->keySz) + (tree->L * (tree->valSz + 1));
}

static BPTNode* bpt_node_alloc(BPlusTree* tree) { 
	BPTNode* n = calloc(1, sizeof(*n) + bpt_node_data_size(tree));
	n->type = 1;
	return n;
}
static BPTNode* bpt_leaf_alloc(BPlusTree* tree) { 
	BPTNode* n = calloc(1, sizeof(*n) + bpt_leaf_data_size(tree));
	n->type = 0;
	return n;
}

static void bpt_free_leaf(BPlusTree* tree, BPTNode* n, int free_data) {
	int i;
	
	if(free_data) {
		for(i = 0; i < n->fill; i++) {
			free(*bpt_get_leaf_val(tree, n, i));
		}
	}
	
	free(n);
}

static void bpt_free_node(BPlusTree* tree, BPTNode* n, int free_data) {
	int i;
	
	if(bpt_is_leaf(n)) {
		bpt_free_leaf(tree, n, free_data);
		return;
	}
	
	for(i = 0; i < n->fill; i++) {
		bpt_free_node(tree, bpt_get_node_ptr(tree, n, i), free_data);
	}
	
	free(n);
}




/////////// internal operations //////////////////

// returns the index within *out_n
static int bpt_find_key_index_leaf(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode** out_n) {
	int i;
	
	*out_n = n;
	
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_leaf_key(tree, n, i);
		if(key < k) {
			return i;
		}
	}
	
	// it's the last index
	return n->fill;
}

// returns the index within *out_n
// recurses down to a leaf.
static int bpt_find_key_index(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode** out_n) {
	int i;
	
	if(bpt_is_leaf(n)) {
		return bpt_find_key_index_leaf(tree, n, key, out_n);
	}
	
	last = 0;
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_node_key(tree, n, i);
		// WARNING: may be broken
		if(k > key) {
			return bpt_find_key_index(tree, bpt_get_node_ptr(tree, n, i), key, out_n);
		}
	}
	
	// follow the tailing pointer
	return bpt_find_key_index(tree, bpt_get_node_ptr(tree, n, n->fill), key, out_n);
}


static void bpt_make_room_node(BPlusTree* tree, BPTNode* n, int index) {
	void* src, *dest;
	size_t size;
	
	src = bpt_get_node_index(tree, n, index);
	dest = bpt_get_node_index(tree, n, index + 1);
	
	size = (n->fill - index) * (tree->keySz + sizeof(BPTNode*));
	
	memmove(dest, src, size);
}

static void bpt_make_room_leaf(BPlusTree* tree, BPTNode* n, int index) {
	void* src, *dest;
	size_t size;
	
	src = bpt_get_leaf_index(tree, n, index);
	dest = bpt_get_leaf_index(tree, n, index + 1);
	
	size = (n->fill - index) * (tree->keySz + tree->valSz);
	
	memmove(dest, src, size);
}

static inline void bpt_put_val_leaf(BPlusTree* tree, BPTNode* n, int index, bpt_key_t key, void* val) {
	void* p = bpt_get_leaf_index(tree, n, index);
	*((bpt_key_t*)p) = key;
	memcpy(p + tree->keySz, val, tree->valSz);
}


static void bpt_node_find_key_index(BPlusTree* tree, BPTNode* n, bpt_key_t key) {
	int i;

	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_node_key(tree, n, i);
		if(k > key) {
			return i - 1;
		}
	}
	
	return n->fill - 1;
}


// inserts a key/child into a node. 
// does not check if it should be there. will split and update parents
static void bpt_insert_node(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode* child) {
	int i;
	int index;
	
	
	// check if we are adding a new root node
	if(n == NULL) {
		
		
		
	}

	//bpt_key_t lowest_r = bpt_get_leaf_key(tree, right, 0);
	//bpt_insert_node()
	
	if(bpt_is_full(n)) {
		// split node
		bpt_split_node(tree, n, key);
		// TODO: finish
	}
	
// 	// the node has room. the overall insertion process ends here
	
	// find the spot
	index = n->fill;
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_node_key(tree, n, i);
		if(k > key) {
			index = i;
			break;
		}
	}
	
	// insert the key and pointer
	bpt_make_room_node(tree, n, index);
	void* p = bpt_get_node_index(tree, n, index);
	*((bpt_key_t*)p) = key;
	*((BPTNode*)(p + sizeof(bpt_key_t))) = child;
}

// sets the children's parent pointer to the parent.
// used after a node split
static void bpt_update_children_parent(BPlusTree* tree, BPTNode* parent) {
	int i;
	
	// careful of off-by-one. there are more pointers than keys 
	for(i = 0; i <= parent->fill; i++) {
		BPTNode* child = bpt_get_node_ptr(tree, parent, i);
		child->parent = parent;
	}
}

// returns the new right node
static BPTNode* bpt_split_node(BPlusTree* tree, BPTNode* left) {
	BPTNode* right;
	
	right = bpt_node_alloc(tree);


// BUG: properly choose the middle value
	int index = left->fill / 2;
	int toleft = index;
	int toright = left->fill - toleft - 1;
	int rindex = index + 1;
	
	// keep the middle key
	bpt_key_t middle_key = bpt_get_node_key(tree, left, index + 1);
	
	// the left stays normal. the right is coppied
	right->fill = toright;
	right->parent = left->parent;
	
	// copy the rightmost data to the new node
	memcpy(bpt_get_pairs(tree, right), bpt_get_node_index(tree, left, toleft + 1), toright * (tree->keySz + sizeof(BPTNode*)));
	
	// update right's children's parent node pointer
	bpt_update_children_parent(tree, right);
	
	// insert the middle key into the parent node
	// check if it's the root and update the tree
	bpt_insert_node(tree, left->parent, middle_key, right);
	
	return right;
}

// returns the new right leaf
static BPTNode* bpt_split_leaf(BPlusTree* tree, BPTNode* left, bpt_key_t hint_key) {
	
	BPTNode* right;
	
	right = bpt_leaf_alloc(tree);
	
	int toleft = left->fill / 2;
	int toright = left->fill - toleft;
	// TODO: shift the odd extra difference based on hint_key
	
	// the left stays normal. the right is coppied
	right->fill = toright;
	right->parent = left->parent;
	
	// copy the rightmost data to the new node
	memcpy(bpt_get_pairs(tree, right), bpt_get_leaf_index(tree, left, toleft + 1), toright * tree->pairSz);
	
	// fix the linked list
	// the previous node still points to the left node
	BPTNode** lptr = bpt_get_leaf_ptr(tree, left);
	BPTNode* rightright = *lptr;
	BPTNode** rptr = bpt_get_leaf_ptr(tree, right);
	*rptr = rightright;
	*lptr = right;
	
	return right;
}



/////////// external operations //////////////////

// tree creation

BPlusTree* bpt_alloc(unsigned short N, unsigned short L) {
	BPlusTree* tree;
	bpt_init(tree, N, L);
	return tree;
}

void bpt_init(BPlusTree* tree, unsigned short N, unsigned short L) { 

	tree->keySz = sizeof(bpt_key_t);
	tree->valSz = sizeof(bpt_val_t);
	
	tree->pairSz = tree->keySz + tree->valSz;
	tree->N = N;
	tree->L = L;

	tree->root = _bpt_node_alloc(tree, 0); // 0 is leaf, 1 is internal
}

void bpt_free(BPlusTree* tree, int free_data) {
	bpt_free_node(tree, tree->root, free_data);
	tree->root = NULL;
}

// data operations


void bpt_insert(BPlusTree* tree, bpt_key_t key, void* val) { 
	int index;
	BPTNode* node;
	
	// find key location
	index = bpt_find_key_index(tree, tree->root, key, &node);
	
	// insert value or split leaf
	if(n->fill < tree->L) {
		bpt_make_room_leaf(tree, n, index);
		bpt_put_val_leaf(tree, n, index, key, val);
		
		// TODO: walk up tree, updating nodes
		
	}
	else {
		// split leaf and grow tree
		printf("!!! BPT insert/split leaf not fully implemented\n");
		
		BPTNode* right = bpt_split_leaf(tree, node, key);
		
		// insert the data into the correct node
		BPTNode* nn = key < bpt_get_leaf_key(tree, right, 0) ? node : right;
		int i2 = bpt_find_key_index_leaf(tree, nn, key, &nn);
		bpt_make_room_leaf(tree, nn, i2);
		bpt_put_val_leaf(tree, nn, i2, key, val);
		
		// update parent node
		bpt_key_t lowest_r = bpt_get_leaf_key(tree, right, 0);
		bpt_insert_node(tree, nn->parent, lowest_r);
	}
	
}

static void bpt_update_lowest(BPlusTree* tree, BPTNode* n, bpt_key_t key) {
	
}



int bpt_find(BPlusTree* tree, bpt_key_t key, void** val) { 
	int index;
	BPTNode* node;
	bpt_key_t k;
	
	index = bpt_find_key_index(tree, tree->root, key, &node);
	
	k = bpt_get_key_leaf(tree, node, index);
	if(k == key) {
		memcpy(val, bpt_get_val_leaf(tree, node, index), tree->valSz);
		return 0;
	}
	
	// not found
	return 1;
}

int bpt_delete(BPlusTree* tree, bpt_key_t key) { 
	
	
}











void bpt_insert_internal(BPTNode* node, BPlusTree* tree, bpt_key_t key, bpt_val_t val) { 
	int i;
	
	if(node->type == 0) {
		bpt_insert_leaf(node, tree, key, val);
		return;
	}
	
	
	for(i = 0; i < tree->fill; i++) {
		if(key < KEY(tree, node, i)) {
			bpt_insert_internal((BPTNode*)DATA(tree, node, i), tree, key, val);
			return;
		}
	}
	
	bpt_insert_internal((BPTNode*)DATA(tree, node, tree->fill), tree, key, val);
}



static void make_room(BPTNode* node, BPlusTree* tree, int index) {
	
	
}

static int find_internal_index(BPTNode* node, BPlusTree* tree, btp_key_t key) {
	
}

static int find_leaf_index(BPTNode* node, BPlusTree* tree, btp_key_t key) {
	int i, index;
	
	index = 0;
	for(i = 0; i < tree->fill; i++) {
		if(key < KEY(tree, node, i)) {
			index = i;
		}
		else {
			break;
		}
	}
	
	
}


void bpt_insert_leaf(BPTNode* node, BPlusTree* tree, bpt_key_t key, bpt_val_t val) { 
	int i, index;

	
	
	
	
	
}

void bpt_insert(BPlusTree* tree, bpt_key_t key, bpt_val_t val) { 
	if()
	
}





BPlusTree* bpt_alloc(unsigned short N, unsigned short L) {
	BPlusTree* tree;
	bpt_init(tree, N, L);
	return tree;
}

void bpt_init(BPlusTree* tree, unsigned short N, unsigned short L) { 
	
	
	tree->keySz = sizeof(bpt_key_t);
	tree->valSz = sizeof(bpt_val_t);
	
	tree->pairSz = tree->keySz + tree->valSz;
	tree->N = N;
	tree->L = L;

	tree->root = _bpt_node_alloc(tree, 0); // 0 is leaf, 1 is internal
}























