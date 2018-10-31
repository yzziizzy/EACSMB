#ifndef __EACSMB_btree_h__
#define __EACSMB_btree_h__

#include <stdint.h>
#include <string.h>
#include <stdio.h>


typedef uint32_t bpt_key_t;
typedef void* bpt_val_t;



typedef struct BPTNode {
	unsigned int type: 1; // 0 is leaf, 1 is internal node
	unsigned int fill: 31; // fill is the number of *keys* 
	struct BPTNode* parent;

// effective structure:
// 	
// 	struct {
// 		bpt_key_t key;
// 		bpt_val_t val;
// 	} pairs[N/L];
//
// 	struct BPTNode* next;

	char d[0];
	
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
	
} BPlusTree;


// temp
void print_structure(BPlusTree* tree) ;




BPlusTree* bpt_alloc(unsigned short N, unsigned short L);
void bpt_init(BPlusTree* tree, unsigned short N, unsigned short L);
void bpt_free(BPlusTree* tree, int free_data);

void bpt_insert(BPlusTree* tree, bpt_key_t key, void* val);


// finds a key. returns 1 if found, 0 if not found
int bpt_find(BPlusTree* tree, bpt_key_t key, void** val);
// like find but also returns info for iteration
int bpt_find_iter(BPlusTree* tree, bpt_key_t key, void** val, int* iter, BPTNode** node);


// finds the lowest key in the tree
// returns 1 if a key is found, 0 if there's no more data
int bpt_first(BPlusTree* tree, BPTNode** node, int* iter, bpt_key_t* key, void** val);
// finds the next key
// returns 1 if a key is found, 0 if there's no more data 
int bpt_next(BPlusTree* tree, BPTNode** node, int* iter, bpt_key_t* key, void** val);
// seeks forward along the bottom of the tree until a matching key is found
int bpt_seek(BPlusTree* tree, BPTNode** node, int* iter, bpt_key_t searchKey, bpt_key_t* key, void** val);


int bpt_delete(BPlusTree* tree, bpt_key_t key);





/////////// temporary reference /////////////////
#if (0 == 1)

static inline void* bpt_get_pairs(BPlusTree* tree, BPTNode* n) {

static void bpt_make_room_leaf(BPlusTree* tree, BPTNode* n, int index) {
static void bpt_make_room_node(BPlusTree* tree, BPTNode* n, int index) {

static int bpt_find_key_index(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode** out_n) {
static int bpt_find_key_index_leaf(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode** out_n) {

static void bpt_free_node(BPlusTree* tree, BPTNode* n, int free_data) {

static void bpt_free_leaf(BPlusTree* tree, BPTNode* n, int free_data) {
static BPTNode* bpt_leaf_alloc(BPlusTree* tree) { 
static BPTNode* bpt_node_alloc(BPlusTree* tree) { 
static inline size_t bpt_leaf_data_size(BPlusTree* tree) {
static inline size_t bpt_node_data_size(BPlusTree* tree) {


static inline BPTNode** bpt_get_leaf_ptr(BPlusTree* tree, BPTNode* n) {

static inline void* bpt_seek_leaf_index(BPlusTree* tree, BPTNode* n, void* p, int seek) {
static inline void* bpt_get_leaf_val(BPlusTree* tree, BPTNode* n, int index) {
static inline bpt_key_t bpt_get_leaf_key(BPlusTree* tree, BPTNode* n, int index) {
static inline void* bpt_get_leaf_index(BPlusTree* tree, BPTNode* n, int index) {

static inline void bpt_put_val_leaf(BPlusTree* tree, BPTNode* n, int index, bpt_key_t key, void* val) {


static inline void* bpt_seek_node_index(BPlusTree* tree, BPTNode* n, void* p, int seek) {
static inline BPTNode* bpt_get_node_ptr(BPlusTree* tree, BPTNode* n, int index) {
static inline bpt_key_t bpt_get_node_key(BPlusTree* tree, BPTNode* n, int index) {
static inline void* bpt_get_node_index(BPlusTree* tree, BPTNode* n, int index) {
static void bpt_node_find_key_index(BPlusTree* tree, BPTNode* n, bpt_key_t key) {


static inline int bpt_is_full(BPlusTree* tree, BPTNode* n) {
static inline int bpt_is_internal(BPTNode* n) {
static inline int bpt_is_leaf(BPTNode* n) {



static BPTNode* bpt_split_node(BPlusTree* tree, BPTNode* left) {
static BPTNode* bpt_split_leaf(BPlusTree* tree, BPTNode* left, bpt_key_t hint_key) {



static void bpt_insert_node(BPlusTree* tree, BPTNode* n, bpt_key_t key, BPTNode* child) {
static void bpt_node_find_key_index(BPlusTree* tree, BPTNode* n, bpt_key_t key) {
static void bpt_update_children_parent(BPlusTree* tree, BPTNode* parent) {

#endif





#endif // __EACSMB_btree_h__
