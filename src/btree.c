
#include <stdlib.h>

#include "btree.h"






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

	return n->fill >= tree->N;
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

	// leaf organization: 
	// tree->L = number of keys
	// val[0] | key[0] | val[1] | key[1] | ... | val[L-1] | key[L-1] | BPTNode* next
static inline BPTNode** bpt_get_leaf_next_ptr(BPlusTree* tree, BPTNode* n) {
	void* p = bpt_get_pairs(tree, n);
	return p + ((n->fill + 1) * (sizeof(bpt_key_t) + tree->valSz));
// 	return (BPTNode**)bpt_get_leaf_index(tree, n, n->fill);
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
	BPTNode* n = calloc(1, sizeof(*n) + bpt_node_data_size(tree) + 60); // TODO: voodoo
	n->type = 1;
	return n;
}
static BPTNode* bpt_leaf_alloc(BPlusTree* tree) { 
	BPTNode* n = calloc(1, sizeof(*n) + bpt_leaf_data_size(tree) + 50); // TODO: voodoo
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


#define MIN(a,b) (a > b ? b : a)


void  _print_node(BPlusTree* tree, BPTNode* n, int indent) {
	int i;
	
	printf("%*scontents [%d] of node %p (parent %p)\n", indent, " ", n->fill, n, n->parent);
	
	for(i = 0; i < MIN(n->fill, 30); i++) {
		bpt_key_t k = bpt_get_node_key(tree, n, i);
		BPTNode* p = bpt_get_node_ptr(tree, n, i);
		if(!bpt_is_leaf(p)) {
			printf("%*s> %d: %p\n", indent, " ", k, p);
		}
		else {
			printf("%*s> [LEAF] %d: %p -> %p (parent %p)\n", indent, " ", k, p, *bpt_get_leaf_next_ptr(tree, p), p->parent);
		}
	}
	
	BPTNode* p = bpt_get_node_ptr(tree, n, n->fill);
// 	printf("> -: %p\n", p);
	if(!bpt_is_leaf(p)) {
		printf("%*s> -: %p\n", indent, " ", p);
	}
	else {
		printf("%*s> [LEAF] -: %p -> %p\n", indent, " ", p, *bpt_get_leaf_next_ptr(tree, p));
	}
		
	printf("%*s>----------------------\n", indent, " ");
}
void _print_leaf(BPlusTree* tree, BPTNode* n, int indent) { 
	int i;
	
	printf("%*scontents [%d] of leaf %p (parent %p)\n%*s> ", indent, " ", n->fill, n, n->parent, indent, " ");
	
	
	for(i = 0; i < MIN(n->fill, 30); i++) {
		bpt_key_t k = bpt_get_leaf_key(tree, n, i);
		uint64_t v = *((uint64_t*)bpt_get_leaf_val_p(tree, n, i));
		printf("%d=%ld, ", k, v);
	}
	
	
	BPTNode** pp = bpt_get_leaf_next_ptr(tree, n);
	printf("\n%*s-next-> %p\n%*s>----------------------\n", indent, " ", *pp, indent, " ");
}

void print_leaf(BPlusTree* tree, BPTNode* n) { return;
	_print_leaf(tree, n, 0);
}
void print_node(BPlusTree* tree, BPTNode* n) { return;
	_print_node(tree, n, 0);
}

void _print_structure(BPlusTree* tree, BPTNode* n, int indent) {
	if(bpt_is_leaf(n)) return;
	
	_print_node(tree, n, indent + 2);
	
	int i;
	for(i = 0; i < n->fill + 1; i++) {
//		bpt_key_t k = bpt_get_node_key(tree, n, i);
		BPTNode* p = bpt_get_node_ptr(tree, n, i);
		_print_structure(tree, p, indent + 2);
	}
} 

void print_structure(BPlusTree* tree) {  return;
	printf("\nROOT: ");
	_print_structure(tree, tree->root, 0);
	printf("\n");
}


void _print_deep_structure(BPlusTree* tree, BPTNode* n, int indent) {
	if(bpt_is_leaf(n)) {
		
		_print_leaf(tree, n, indent + 2);
		return;
	};
	
	_print_node(tree, n, indent + 2);
	
	int i;
	for(i = 0; i < n->fill + 1; i++) {
//		bpt_key_t k = bpt_get_node_key(tree, n, i);
		BPTNode* p = bpt_get_node_ptr(tree, n, i);
		_print_deep_structure(tree, p, indent + 2);
	}
} 

void print_deep_structure(BPlusTree* tree) {  return;
	printf("\nROOT: ");
	_print_deep_structure(tree, tree->root, 0);
	printf("\n");
}


/////////// internal operations //////////////////
static BPTNode* bpt_split_node(BPlusTree* tree, BPTNode* left, bpt_key_t* middle_key);



// returns the index within *out_n
static int bpt_find_key_index_leaf(BPlusTree* tree, BPTNode* n, bpt_key_t key) {
	int i;
	
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_leaf_key(tree, n, i);
		if(key <= k) {
			return i;
		}
	}
	
	// it's the last index
	return n->fill;
}

// returns the leaf that a key should live in 
// recurses down to a leaf.
static BPTNode* bpt_find_key_leaf_node(BPlusTree* tree, BPTNode* n, bpt_key_t key) {
	int i;
	
	if(bpt_is_leaf(n)) {
		return n;
	}
	
	for(i = 0; i < n->fill; i++) {
		bpt_key_t k = bpt_get_node_key(tree, n, i);
		// WARNING: may be broken
		if(k > key) {
			break;
		}
	}
	
	return bpt_find_key_leaf_node(tree, bpt_get_node_ptr(tree, n, i), key);
}

// returns the index within *out_n
// recurses down to a leaf.
static int bpt_find_key_index(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode** out_n) {
	int i, last;
	
	if(bpt_is_leaf(n)) {
		*out_n = n;
		return bpt_find_key_index_leaf(tree, n, key);
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
	size = ((n->fill - index + 1) * (tree->keySz + sizeof(BPTNode*)));
	
	//printf("make_room_node: index: %d, size: %d\n", index, size);
	
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
static void bpt_insert_key_into_node(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode* child) {
	int i;
	int index;
	BPTNode* new_root;
	

	//bpt_key_t lowest_r = bpt_get_leaf_key(tree, right, 0);
	//bpt_insert_node()
	
	if(!bpt_is_full(tree, n)) {
		
		// find the spot
		for(i = 0; i < n->fill; i++) {
			bpt_key_t k = bpt_get_node_key(tree, n, i);
			if(k > key) {
				break;
			}
		}
		
		// insert the key and pointer
		bpt_make_room_node(tree, n, i); 
		bpt_key_t* k = bpt_get_node_key_p(tree, n, i); // BUG: old algorithm had OBO hack here 
		BPTNode** p = bpt_get_node_ptr_p(tree, n, i+1);
		*k = key;
		*p = child;
		n->fill++;
		
		return;
	}
	else {
		// split node
	//	printf("splitting node %p\n", n);
		
	//	printf("F: "); print_deep_structure(tree);
		
		bpt_key_t middle_key;
		BPTNode* right = bpt_split_node(tree, n, &middle_key);
		
		
	//	printf("G: "); print_deep_structure(tree);
		
		
		// splitting the root node
		if(n->parent == NULL) {
	//		printf("new root node (at node), middle key %d\n", middle_key);
			new_root = bpt_node_alloc(tree);
			
			// put in the two pointers
			BPTNode** p = bpt_get_node_ptr_p(tree, new_root, 0);
			*p = n;
			p = bpt_get_node_ptr_p(tree, new_root, 1);
			*p = right;
			
			// put in the middle key
			bpt_key_t* k = bpt_get_node_key_p(tree, new_root, 0);
			*k = middle_key;
			
			new_root->fill = 1;
			n->parent = new_root;
			right->parent = new_root;
			tree->root = new_root;
			
			// recurse to handle the original insertion
			BPTNode* nn;
			if(key < middle_key) {
				nn = n;
			}
			else {
				nn = right;
			}
			
			bpt_insert_key_into_node(tree, nn, key, child);
			return;
		}
		
		
		// splitting a regular node
		
		bpt_insert_key_into_node(tree, n->parent, middle_key, right);
		right->parent = n->parent;
		
		bpt_insert_key_into_node(tree, key < middle_key ? n : right, key, child);
		return;
	}

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
static BPTNode* bpt_split_node(BPlusTree* tree, BPTNode* left, bpt_key_t* middle_key) {
	BPTNode* right;
	
	right = bpt_node_alloc(tree);


// BUG: properly choose the middle value
	
	// these are key indices
	int index = left->fill / 2;
	int toleft = index + 1;
	int toright = left->fill - toleft;
	int rindex = index + 1;
	
	// keep the middle key
	*middle_key = bpt_get_node_key(tree, left, index);
	
	
//	printf(" node split: tl:%d, tr: %d, rindex:%d mk:%d \n", toleft, toright, rindex, *middle_key);
	
	// the left stays normal. the right is copied
	left->fill = toleft - 1; // middle key is excluded
	right->fill = toright;
	right->parent = left->parent;
	
	// TODO: double check
	

	
	// copy the rightmost data to the new node
	//bpt_get_node_ptr_p(tree, left, toleft)
	
	memcpy(
		bpt_get_pairs(tree, right), 
		bpt_get_node_ptr_p(tree, left, toleft), 
		((toright) * (tree->keySz + sizeof(BPTNode*))) + sizeof(BPTNode*)
	);
	
	
//	printf("Left:");
	print_node(tree, left);
//	printf("Right:");
	print_node(tree, right);
	
	
	// update right's children's parent node pointer
	//bpt_update_children_parent(tree, left);
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
	
	//printf("splitting leaf %p into %p\n", left, right);
	
	
	int toleft = left->fill / 2;
	int toright = left->fill - toleft;
	// TODO: shift the odd extra difference based on hint_key
	
	// the left stays normal. the right is coppied
	left->fill = toleft;
	right->fill = toright;
	right->parent = left->parent;
	
	//printf(" leaf split data: tr:%d, tl:%d, parent: %p\n", toleft, toright, left->parent);
	// TODO: double check
	
	// copy the rightmost data to the new node
	memcpy(bpt_get_pairs(tree, right), bpt_get_leaf_index(tree, left, toleft), (toright) * tree->pairSz);
	
	print_leaf(tree, left);
	print_leaf(tree, right);
	
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


static void bpt_insert_leaf_non_full(BPlusTree* tree, BPTNode* node, bpt_key_t key, void* val) {
	
	int index = bpt_find_key_index_leaf(tree, node, key);
	
	bpt_make_room_leaf(tree, node, index);
	bpt_put_val_leaf(tree, node, index, key, val);
	node->fill++;
}


/////////// external operations //////////////////

// tree creation

BPlusTree* bpt_alloc(unsigned short N, unsigned short L, size_t valSz) {
	BPlusTree* tree;
	
	tree = calloc(1, sizeof(*tree));
	tree->valSz = valSz;
	
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
	
	//printf("\n\n@@@@@@@@@@@@@@@@@@@@@\ninserting key: %d\n", key);
	print_structure(tree);
	// find key location
	//index = bpt_find_key_index(tree, tree->root, key, &node);
	
	node = bpt_find_key_leaf_node(tree, tree->root, key);
	
	// insert value or split leaf
	if(node->fill < tree->L) {
		bpt_insert_leaf_non_full(tree, node, key, val);
		//print_leaf(tree, node);
		/*
		A new lowest value will never be inserted into a leaf that's under an internal node.
		The only case where this can happen is if the leaf is the root node, where updating
		  the parent has no meaning.
		*/
	}
	else {
		// split leaf and grow tree
	//	printf("split leaf %d\n", node->fill);
		
		right = bpt_split_leaf(tree, node, key);
		
		// splitting a solo leaf root node
		if(node->parent == NULL) {
			
		//	printf("new root at leaf, splitting on %d\n", bpt_get_leaf_key(tree, right, 0));
			new_root = bpt_node_alloc(tree);
			
			// put in the two pointers
			BPTNode** p = bpt_get_node_ptr_p(tree, new_root, 0);
			*p = node;
			p = bpt_get_node_ptr_p(tree, new_root, 1);
			*p = right;
			
			// put in the lowest right key
			bpt_key_t* k = bpt_get_node_key_p(tree, new_root, 0);
			*k = bpt_get_leaf_key(tree, right, 0);
			
			new_root->fill = 1;
			tree->root = new_root;
			node->parent = new_root;
			right->parent = new_root;
			
			
			
			// recurse for the original insertion
			bpt_insert(tree, key, val);
			return;
		}
		
		
		// a normal split is happening, under a node
		bpt_key_t lowest_r = bpt_get_leaf_key(tree, right, 0);
		
		bpt_insert_key_into_node(tree, node->parent, lowest_r, right);
		right->parent = node->parent;
		
	//	printf("Pre re-insert: \n");
		print_deep_structure(tree);
		
		// recurse for the original insertion
		bpt_insert(tree, key, val);
		return;
	}
	
	
	// TODO: fix
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




// merges the right node into the left node, under the same parent
static BPTNode* bpt_merge_leaves(BPlusTree* tree, BPTNode* l, BPTNode* r) {
	BPTNode** l_next;
	void* r_src, *l_dest;
	size_t size;
	
// 	_print_leaf(tree, r, 5);
// 	_print_leaf(tree, l, 5);
	
	
	// TODO: test and swap l and r
	if(*bpt_get_leaf_next_ptr(tree, l) != r) {
		BPTNode* tmp = l;
		l = r;
		r = tmp;
	}
	
	// append the right node's keys into the left node
	r_src = bpt_get_leaf_index(tree, r, 0);
	l_dest = bpt_get_leaf_index(tree, l, l->fill);
		
	size = ((r->fill) * (tree->keySz + tree->valSz)) + sizeof(BPTNode*);
		//printf("make_room_leaf: %d\n", size);
	memmove(l_dest, r_src, size);
		
	l->fill += r->fill;
	
	// set the left's list pointer to the right's list pointer 
	l_next = bpt_get_leaf_next_ptr(tree, l);
	*l_next = *bpt_get_leaf_next_ptr(tree, r);
	
	// clear the right node's fill
	r->fill = 0; // unnecessary?
	
	// caller must free the right node
	
	return r;
}


static BPTNode* bpt_get_immediate_sibling(BPlusTree* tree, BPTNode* n) {
	BPTNode* sib;
	BPTNode* parent;
	
	sib = *bpt_get_leaf_next_ptr(tree, n);
	printf("  sib: %p, n: %p\n", sib, n); 
	
	
	
	// easy case, return early
	if(sib->parent == n->parent) return sib;
	printf("  sib->parent: %p, n->parent %p \n", sib->parent, n->parent);
	
	printf("\nn:\n");
	_print_leaf(tree, n, 5);
	printf("\nsib:\n");
 	_print_leaf(tree, sib, 5);
	printf("\nn->parent:\n");
	_print_node(tree, n->parent, 5);

	
	// n is the right-most sibling in the parent node
	parent = n->parent;
	
	sib = bpt_get_node_ptr(tree, parent, parent->fill - 1); // BUG: this index might be off
	printf("  sib from parent: %p\n ", sib);
	// this should be true if n is not an only child
	if(*bpt_get_leaf_next_ptr(tree, sib) == n) return sib;
	
	// hopefully the b+ tree's definition keeps us from getting here.
	// BUG: we shall see
	printf("failure to find sibling in %s\n", __func__);
	return NULL;
} 

// called after a leaf merge
static void bpt_repair_node_keys(BPlusTree* tree, BPTNode* n) {
	printf("  repairing node\n");
	//_print_node(tree, n, 5);
	
	for(int i = 0; i < n->fill; i++) {
		BPTNode* p = bpt_get_node_ptr(tree, n, i + 1);
		bpt_key_t* kp = bpt_get_node_key_p(tree, n, i);
		
		*kp = bpt_get_leaf_key(tree, p, 0);
	}
	
	
	_print_node(tree, n, 5);
	
	printf("  repnode^^^^^^^^^^^^^^^\n");
// 	printf("NIH: bpt_repair_node_keys\n");
	//exit(1);
}


// 1. if a leaf is not empty, delete the key and move the data down
// 2. if a leaf runs out of keys, delete it
//    a. remove key from parent node

// 3. if a node runs out of keys, merge with a sibling
//    a. find a sibling
//       ii. look to the parent node for a previous sibling
//       iii. return NULL, meaning no sibling left
//    b. copy half the keys from the sibling
//    c. update child parent pointers for both

// 4. if a node runs out of keys, pull a key/pointer down from the parent to fill it in 


// n. when the second to last node runs out of keys, leaving the parent with only one child,
//      bring a key down from the parent
// n+!. when the root runs out of keys delete it and make the single remaining child the new root


int bpt_delete(BPlusTree* tree, bpt_key_t key) { 
	int index;
	BPTNode* n;
	bpt_key_t k;
	
	printf("!!! debug return in bpt_delete\n");
	return 0;
	
	index = bpt_find_key_index(tree, tree->root, key, &n);
	
	k = bpt_get_leaf_key(tree, n, index);
	if(k != key) {
		printf("bpt_delete: key not found %d, %d, %d\n", key, index, k);
		return 0;
	}
	
	// the key is in the node
	
	// check if this is the last one
	if(n->fill == 1) { // 2. if a leaf runs out of keys, merge with a sibling
		printf(" delete: (%d) merging with sibling\n", key);
		// try merge with sibling
		BPTNode* sibling = bpt_get_immediate_sibling(tree, n);
		if(!sibling) {
			printf("failed to get sibling in %s\n", __func__);
			
			// ???
			// merge parent and sibling
			
		}
		
		// returns the node to delete
		BPTNode* dead = bpt_merge_leaves(tree, n, sibling);
		free(dead);
		
		bpt_repair_node_keys(tree, dead == n ? sibling->parent : n->parent);
	}
	else { // 1. if a leaf is not empty, delete the key and move the data down
		printf(" delete: (%d) moving items down\n", key);
		// move other items down and decrement fill
		void* src, *dest;
		size_t size;
		
		_print_leaf(tree, n, 4);
		
		src = bpt_get_leaf_index(tree, n, index + 1);
		dest = bpt_get_leaf_index(tree, n, index);
		
		// TODO:                                                        fix this voodoo
		size = ((n->fill - index - 1) * (tree->keySz + tree->valSz)) + (2*sizeof(BPTNode*));
		//printf("make_room_leaf: %d\n", size);
		memmove(dest, src, size);
		
		
		
		n->fill--;
		
		_print_leaf(tree, n, 4);
		
		printf(" del^^^^^^^^^^^^^^^\n");
		
		// new lowest keys don't need to be propagated 
		//   upward because the old value still works
		
	}
	
	return 0;
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
	
//	printf(" first %d %p %p \n", *key, n, node);
	
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
	if(index >= n->fill) {
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
// the search is abandoned at the first greater key
int bpt_seek(BPlusTree* tree, BPTNode** node, int* iter, bpt_key_t searchKey, bpt_key_t* key, void** val) { 
	int index = *iter;
	BPTNode* n = *node;
	bpt_key_t k;
	
	if(tree->size == 0) return 0;

	//printf("index: %d, searchKey: %d\n", index, searchKey);
	
	
	if(index == -1) { // first step
		n = tree->root;
		while(n) { // walk down the left side
			if(bpt_is_leaf(n)) break;
			n = bpt_get_node_ptr(tree, n, 0);
		}
		index = 0;
		*node = n;
	}
	else {
		index++;
	}
	
	
	//	printf("index: %d\n", index);
// 	index = index == -1 ? 0 : index + 1;
	
	////printf("index: %d\n", index);
	while(1) {
	//	printf("node1: %p\n", n);
		
		// hop to next leaf
		if(index > n->fill) {
			n = *bpt_get_leaf_next_ptr(tree, n);
			if(n == NULL) {
	//			printf("key seek failed 2\n\n");
				return 0;
			}
			
	//		printf("next node %p \n", n);
			
	//		print_deep_structure(tree);
			
			*node = n;
			index = 0;
		}
	//	printf("node2: %p\n", n);
		k = bpt_get_leaf_key(tree, n, index); 
		
	//	print_leaf(tree, n);
	//	printf("&> %d %d\n", k, index);
		if(k >= searchKey) {
			if(k == searchKey) break;
			
	//		printf("key seek failed\n\n");
			return 0;
			
		}
		
		index++;
	}
	
	*key = k;
	*val = bpt_get_leaf_val_p(tree, n, index);
	*iter = index;
	//printf("***************found key\n\n");
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





















