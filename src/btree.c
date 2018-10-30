
#include <stdlib.h>

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
		return n->fill >= tree->L + 1;
	}

	return n->fill >= tree->N + 1;
}

//////////////////

	// node organization: 
	// ptr[0] | key[0] | ptr[1] | key[1] | ... | key[N-2] | ptr[N-1]
	
static inline void* bpt_get_node_index(BPlusTree* tree, BPTNode* n, int index) {
	void* p = bpt_get_pairs(tree, n);
	return p + ((tree->keySz + sizeof(BPTNode*)) * index);
} // ?

static inline bpt_key_t* bpt_get_node_key_p(BPlusTree* tree, BPTNode* n, int index) {
	return (bpt_key_t*)(bpt_get_node_index(tree, n, index) + sizeof(BPTNode*));
}//
static inline bpt_key_t bpt_get_node_key(BPlusTree* tree, BPTNode* n, int index) {
	return *bpt_get_node_key_p(tree, n, index);
}//

static inline BPTNode** bpt_get_node_ptr_p(BPlusTree* tree, BPTNode* n, int index) {
	return (BPTNode**)(bpt_get_node_index(tree, n, index) + 0);
}//
static inline BPTNode* bpt_get_node_ptr(BPlusTree* tree, BPTNode* n, int index) {
	return *bpt_get_node_ptr_p(tree, n, index);
}//

static inline void* bpt_seek_node_index(BPlusTree* tree, BPTNode* n, void* p, int seek) {
	return p + ((tree->keySz + sizeof(BPTNode*)) * seek);
}// ? 


////////////////////

	// leaf organization: 
	// val[0] | key[0] | val[1] | key[1] | ... | val[L-1] | key[L-1] | BPTNode* next

static inline void* bpt_get_leaf_index(BPlusTree* tree, BPTNode* n, int index) {
	void* p = bpt_get_pairs(tree, n);
	return p + ((tree->pairSz) * index);
}//

static inline bpt_key_t* bpt_get_leaf_key_p(BPlusTree* tree, BPTNode* n, int index) {
	return (bpt_key_t*)(bpt_get_leaf_index(tree, n, index) + tree->valSz);
}//

static inline bpt_key_t bpt_get_leaf_key(BPlusTree* tree, BPTNode* n, int index) {
	return *bpt_get_leaf_key_p(tree, n, index);
}//

static inline void* bpt_get_leaf_val_p(BPlusTree* tree, BPTNode* n, int index) {
	return bpt_get_leaf_index(tree, n, index) + 0;
}//

static inline void* bpt_seek_leaf_index(BPlusTree* tree, BPTNode* n, void* p, int seek) {
	return p + ((tree->pairSz) * seek);
} // ?

static inline BPTNode** bpt_get_leaf_next_ptr(BPlusTree* tree, BPTNode* n) {
	return (BPTNode**)bpt_get_leaf_index(tree, n, n->fill);
} // updated but might be broken still



////////// allocation ////////////

// TODO: double check and adjust these
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
			free(*((void**)bpt_get_leaf_val_p(tree, n, i)));
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



void print_node(BPlusTree* tree, BPTNode* n) { return;
	int i;
	
	printf("contents [%d] of node %p (parent %p)\n", n->fill, n, n->parent);
	
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_node_key(tree, n, i);
		BPTNode* p = bpt_get_node_ptr(tree, n, i);
		if(!bpt_is_leaf(p)) {
			printf("> %d: %p\n", k, p);
		}
		else {
			printf("> [LEAF] %d: %p\n", k, p);
		}
	}
	
	BPTNode* p = bpt_get_node_ptr(tree, n, n->fill);
// 	printf("> -: %p\n", p);
	if(!bpt_is_leaf(p)) {
		printf("> -: %p\n", p);
	}
	else {
		printf("> [LEAF] -: %p\n", p);
	}
		
	printf(">----------------------\n");
}
void print_leaf(BPlusTree* tree, BPTNode* n) { 
	int i;
	
	printf("contents [%d] of leaf %p\n> ", n->fill, n);
	/*
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_leaf_key(tree, n, i);
		printf("%d ", k);
	}
	*/
	printf(" -> %p\n>----------------------\n", *bpt_get_leaf_next_ptr(tree, n));
}


void _print_structure(BPlusTree* tree, BPTNode* n) {
	if(bpt_is_leaf(n)) return;
	
	print_node(tree, n);
	
	int i;
	for(i = 0; i < n->fill + 1; i++) {
// 		bpt_key_t k = bpt_get_node_key(tree, n, i);
		BPTNode* p = bpt_get_node_ptr(tree, n, i);
		_print_structure(tree, p);
	}
} 

void print_structure(BPlusTree* tree) {  return;
	printf("\nROOT: ");
	_print_structure(tree, tree->root);
	printf("\n");
}


/////////// internal operations //////////////////
static BPTNode* bpt_split_node(BPlusTree* tree, BPTNode* left, bpt_key_t* middle_key);



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
	int i, last;
	
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
//	print_structure(tree);
//	if(index > n->fill + 1) return;
	
	src = bpt_get_node_index(tree, n, index);
	dest = bpt_get_node_index(tree, n, index + 1);
	
	// TODO: double check for odd pairing off-by-one
	size = ((n->fill - index + 1) * (tree->keySz + sizeof(BPTNode*))) + sizeof(BPTNode*);
	
	printf("make_room_node: index: %d, size: %d\n", index, size);
	
	if(size <= 0) return;
	
	memmove(dest, src, size);
}

static void bpt_make_room_leaf(BPlusTree* tree, BPTNode* n, int index) {
	void* src, *dest;
	size_t size;
	
	src = bpt_get_leaf_index(tree, n, index);
	dest = bpt_get_leaf_index(tree, n, index + 1);
	
	// TODO: double check
	size = ((n->fill - index) * (tree->keySz + tree->valSz)) + sizeof(BPTNode*);
	//printf("make_room_leaf: %d\n", size);
	memmove(dest, src, size);
}

static inline void bpt_put_val_leaf(BPlusTree* tree, BPTNode* n, int index, bpt_key_t key, void* val) {
	void* p = bpt_get_leaf_val_p(tree, n, index);
	bpt_key_t* k = bpt_get_leaf_key_p(tree, n, index);
	*k = key;
	memcpy(p, val, tree->valSz);
	
	//printf("  key %d inserted into %p\n", key, n);
	//print_leaf(tree, n);
}


static int bpt_node_find_key_index(BPlusTree* tree, BPTNode* n, bpt_key_t key) {
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
// 		printf("insert node - new root node\n");
// 		n = bpt_node_alloc(tree);
// 		tree->root = n;
// 		child->parent = n;
		
		// the left and right pointers both need to be updated 
		
// 		BPTNode* new_root;
// 		
// 		
// 		new_root = bpt_node_alloc(tree);
// 		BPTNode** p = bpt_get_node_ptr_p(tree, new_root, 0);
// 		*p = n;
// 		p = bpt_get_node_ptr_p(tree, new_root, 1);
// 		*p = right;
// 		tree->root = new_root;
// 		n->parent = new_root;
// 		new_root->fill = 1;
// 
// 		printf("insert node - new root node (at leaf) %p\n", new_root);
// 		
// 		bpt_key_t lowest_r = bpt_get_leaf_key(tree, right, 0);
// 		bpt_key_t* kp = bpt_get_node_key_p(tree, new_root, 0);
// 		*kp = lowest_r;
// 		
		
		printf("!!! inserting into null node\n");
	}

	//bpt_key_t lowest_r = bpt_get_leaf_key(tree, right, 0);
	//bpt_insert_node()
	
	if(bpt_is_full(tree, n)) {
		// split node
		printf("splitting node %p\n", n);
		
		bpt_key_t middle_key;
		BPTNode* right = bpt_split_node(tree, n, &middle_key);
		
		
		BPTNode* new_root;
		if(n == tree->root) {
			
			
			new_root = bpt_node_alloc(tree);
			
			printf("new rood node (at node) %p\n", new_root);
			
			BPTNode** p = bpt_get_node_ptr_p(tree, new_root, 0);
			*p = n;
			p = bpt_get_node_ptr_p(tree, new_root, 1);
			*p = right;
			tree->root = new_root;
			n->parent = new_root;
			right->parent = new_root;
		}
		
	//	bpt_key_t lower_r_key = bpt_get_node_key(tree, right, 0);
		bpt_insert_node(tree, n->parent, middle_key, right);
		
		print_structure(tree);
		return; 
	}

	printf("inserting key %d into non-full node %p\n", key, n);
	
// 	// the node has room. the overall insertion process ends here
	
	// find the spot
	index = n->fill + 1;
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_node_key(tree, n, i);
		if(k > key) {
			index = i;
			break;
		}
	}
	
	// insert the key and pointer
	bpt_make_room_node(tree, n, index); // TODO: this fn needs to move one extra ptr too
	bpt_key_t* k = bpt_get_node_key_p(tree, n, index - 1); // TODO: move OBO hack to the right place
	BPTNode** p = bpt_get_node_ptr_p(tree, n, index);
	*k = key;
	*p = child;
	n->fill++;
	
	print_structure(tree);
}

// sets the children's parent pointer to the parent.
// used after a node split
static void bpt_update_children_parent(BPlusTree* tree, BPTNode* parent) {
	int i;
	
	// careful of off-by-one. there are more pointers than keys 
	for(i = 0; i < parent->fill; i++) {
		BPTNode* child = bpt_get_node_ptr(tree, parent, i);
		child->parent = parent;
	}
}

// returns the new right node
static BPTNode* bpt_split_node(BPlusTree* tree, BPTNode* left, bpt_key_t* middle_key) {
	BPTNode* right;
	
	right = bpt_node_alloc(tree);


// BUG: properly choose the middle value
	int index = left->fill / 2;
	int toleft = index;
	int toright = left->fill - toleft - 1;
	int rindex = index + 1;
	
	// keep the middle key
	*middle_key = bpt_get_node_key(tree, left, index);
	
	
	printf(" node split: tl:%d, tr: %d, rindex:%d mk:%d \n", toleft, toright, rindex, middle_key);
	
	// the left stays normal. the right is copied
	left->fill = toleft;
	right->fill = toright;
	right->parent = left->parent;
	
	// TODO: double check
	
	print_structure(tree);
	
	// copy the rightmost data to the new node
	memcpy(bpt_get_pairs(tree, right), bpt_get_node_index(tree, left, toleft + 1), (toright + 1) * (tree->keySz + sizeof(BPTNode*)));
	
	// update right's children's parent node pointer
	bpt_update_children_parent(tree, right);
	
	print_structure(tree);
	
	// insert the middle key into the parent node
	// check if it's the root and update the tree
// 	bpt_insert_node(tree, left->parent, middle_key, right);
// 	
	return right;
}


// returns the new right leaf
static BPTNode* bpt_split_leaf(BPlusTree* tree, BPTNode* left, bpt_key_t hint_key) {
	
	BPTNode* right;
	
	
	BPTNode** lptr = bpt_get_leaf_next_ptr(tree, left);
	BPTNode* rightright = *lptr;
	
	right = bpt_leaf_alloc(tree);
	
	printf("splitting leaf %p into %p\n", left, right);
	
	
	int toleft = left->fill / 2;
	int toright = left->fill - toleft;
	// TODO: shift the odd extra difference based on hint_key
	
	// the left stays normal. the right is coppied
	left->fill = toleft + 1;
	right->fill = toright - 1;
	right->parent = left->parent;
	
	printf(" leaf split data: tr:%d, tl:%d\n", toleft, toright);
	// TODO: double check
	
	// copy the rightmost data to the new node
	memcpy(bpt_get_pairs(tree, right), bpt_get_leaf_index(tree, left, toleft + 1), (toright + 1) * tree->pairSz);
	
	
	//print_leaf(tree, left);
	//print_leaf(tree, right);
	// fix the linked list
	// the previous node still points to the left node
	lptr = bpt_get_leaf_next_ptr(tree, left);
	BPTNode** rptr = bpt_get_leaf_next_ptr(tree, right);
	*rptr = rightright;
	*lptr = right;
	
	
	
//	print_leaf(tree, left);
//	print_leaf(tree, right);
	
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

	if(!tree->keySz) tree->keySz = sizeof(bpt_key_t);
	if(!tree->valSz) tree->valSz = sizeof(bpt_val_t);
	
	tree->pairSz = tree->keySz + tree->valSz;
	tree->N = N;
	tree->L = L;
	
	tree->root = bpt_leaf_alloc(tree); // 0 is leaf, 1 is internal
}

void bpt_free(BPlusTree* tree, int free_data) {
	bpt_free_node(tree, tree->root, free_data);
	tree->root = NULL;
}

// data operations


void bpt_insert(BPlusTree* tree, bpt_key_t key, void* val) { 
	int index, i2;
	BPTNode* node, *nn, *right, *new_root;
	
	//printf("inserting key: %d\n", key);
	
	// find key location
	index = bpt_find_key_index(tree, tree->root, key, &node);
	
	// insert value or split leaf
	if(node->fill < tree->L) {
		bpt_make_room_leaf(tree, node, index);
		bpt_put_val_leaf(tree, node, index, key, val);
		node->fill++;
		//print_leaf(tree, node);
		/*
		A new lowest value will never be inserted into a leaf that's under an internal node.
		The only case where this can happen is if the leaf is the root node, where updating
		  the parent has no meaning.
		*/
	}
	else {
		// split leaf and grow tree
		printf("split leaf %d\n", node->fill);
		
		right = bpt_split_leaf(tree, node, key);
		
		// insert the data into the correct node
		nn = key < bpt_get_leaf_key(tree, right, 0) ? node : right;
		i2 = bpt_find_key_index_leaf(tree, nn, key, &nn);
		bpt_make_room_leaf(tree, nn, i2);
		bpt_put_val_leaf(tree, nn, i2, key, val);
		nn->fill++;
		
		print_leaf(tree, nn);
		print_structure(tree);
		
		// handle splitting the root node
		if(node->parent == NULL) {
			// insert a new root node
			
			new_root = bpt_node_alloc(tree);
			BPTNode** p = bpt_get_node_ptr_p(tree, new_root, 0);
			*p = node;
			p = bpt_get_node_ptr_p(tree, new_root, 1);
			*p = right;
			tree->root = new_root;
			node->parent = new_root;
			right->parent = new_root;
			new_root->fill = 1;

			printf("insert node - new root node (at leaf) %p\n", new_root);
			
			bpt_key_t lowest_r = bpt_get_leaf_key(tree, right, 0);
			bpt_key_t* kp = bpt_get_node_key_p(tree, new_root, 0);
			*kp = lowest_r;
		//	bpt_insert_node(tree, new_root, lowest_r, right);
			// TODO: fix left and right nodes here
		}
		else {
			bpt_key_t lowest_r = bpt_get_leaf_key(tree, right, 0);
			bpt_insert_node(tree, nn->parent, lowest_r, right);
		}
		
		print_structure(tree);
		//bpt_insert(tree, key, val);
	}
	
	tree->size++;
}


// finds a key. returns 1 if found, 0 if not found
int bpt_find(BPlusTree* tree, bpt_key_t key, void** val) { 
	int index;
	BPTNode* node; // these are just thrown away
	
	return bpt_find_iter(tree, key, val, &index, &node);
}


// like find but also returns info for iteration
int bpt_find_iter(BPlusTree* tree, bpt_key_t key, void** val, int* iter, BPTNode** node) { 
	int index;
	BPTNode* n;
	bpt_key_t k;
	
	index = bpt_find_key_index(tree, tree->root, key, &n);
	
	k = bpt_get_leaf_key(tree, n, index);
	if(k == key) {
		memcpy(val, bpt_get_leaf_val_p(tree, n, index), tree->valSz);
		*node = n;
		*iter = index; 
		return 1;
	}
	
	// not found
	return 0;
	
}


int bpt_delete(BPlusTree* tree, bpt_key_t key) { 
	
	
}


// finds the lowest key in the tree
// returns true if a key is found, 0 if there's no more data
int bpt_first(BPlusTree* tree, BPTNode** node, int* iter, bpt_key_t* key, void** val) { 
	BPTNode* n = tree->root;
//	printf("tree size %d\n", tree->size);
	if(tree->size == 0) return 0;
	
	// walk down the left side
	while(n) {
		if(bpt_is_leaf(n)) break;
		n = bpt_get_node_ptr(tree, n, 0);
	}
	
	*node = n;
	*key = bpt_get_leaf_key(tree, n, 0);
	*val = bpt_get_leaf_val_p(tree, n, 0);
	*((int*)iter) = 0;
	
	//printf(" fisrt %d %p %p \n", *key, n, node);
	
	return 1;
}

// finds the next key
// returns true if a key is found, 0 if there's no more data 
int bpt_next(BPlusTree* tree, BPTNode** node, int* iter, bpt_key_t* key, void** val) { 
	int index = *iter;
	BPTNode* n = *node;
	
	if(!n) return 0;
	
	index++;
	//
	//	printf("%d %d %p \n", index, n->fill, n);
	if(index >= tree->L) {
		n = *bpt_get_leaf_next_ptr(tree, n);
		if(n == NULL) return 0;
		
	//	printf("-\n");
		
		*node = n;
		index = 0;
	} 
	
	*key = bpt_get_leaf_key(tree, n, index);
	*val = bpt_get_leaf_val_p(tree, n, index);
	*iter = index;
	
//	printf("%d ", *key);
	
	return 1;
}


// seeks forward along the bottom of the tree until a matching key is found
// returns true if a key is found, 0 if there's no more data 
int bpt_seek(BPlusTree* tree, BPTNode** node, int* iter, bpt_key_t searchKey, bpt_key_t* key, void** val) { 
	int index = *iter;
	BPTNode* n = *node;
	bpt_key_t k;
	
		printf("index: %d\n", index);
	index = index == -1 ? 0 : index + 1;
	
	printf("index: %d\n", index);
	while(1) {
		
		
		// hop to next leaf
		if(index >= tree->L) {
			if(n == NULL) {
				printf("key seek fialed 2\n\n");
				return 0;
			}
			n = *bpt_get_leaf_next_ptr(tree, n);
			printf("next node \n");
			*node = n;
			index = 0;
		}
		
		k = bpt_get_leaf_key(tree, n, index); 
		print_leaf(tree, n);
		printf("> %d %d %d\n", k, *key, index);
		if(k >= *key) {
			if(k == *key) break;
			
			printf("key seek failed\n\n");
			return 0;
			
		}
		
		index++;
	}
	
	*key = k;
	*val = bpt_get_leaf_val_p(tree, n, index);
	*iter = index;
	printf("found key\n\n");
	return 1;
}





/* appears to be unused

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

*/





















