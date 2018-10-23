#ifndef __EACSMB_btree_h__
#define __EACSMB_btree_h__



typedef uint32_t bpt_key_t;
typedef void* bpt_val_t;



typedef struct BPTNode {
	unsigned int type: 1; // 0 is leaf, 1 is internal node
	unsigned int fill: 31;
	struct BPTNode* parent;

// effective structure:
// 	
// 	struct {
// 		bpt_key_t key;
// 		bpt_val_t val;
// 	} pairs[N/L];
//
// 	struct BPTNode* next;

	char d[0]
	
} BPTNode;




typedef struct BPlusTree {
	
	int size;
	int numNodes;
	int numLeaves;
	
	size_t keySz;
	size_t valSz;
	size_t pairSz;
	
	unsigned short N, L;
	
	BPTNode* root;
	
} BPlusTree








BPlusTree* bpt_alloc();
void bpt_init(BPlusTree* tree); 






#endif // __EACSMB_btree_h__
