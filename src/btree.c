

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
static int bpt_find_key_index(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode** out_n) {
	int i;
	
	if(bpt_is_leaf(n)) {
		return bpt_find_key_index_leaf(tree, n, key, out_n);
	}
	
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_node_key(tree, n, i);
		if(key < k) {
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
	}
	else {
		// split leaf and grow tree
		printf("!!! BPT insert/split leaf not implemented\n");
	}
	
	// walk up tree and propagate new lowest value
	
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























