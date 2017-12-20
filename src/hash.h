#ifndef __EACSMB_HASH_H__
#define __EACSMB_HASH_H__

#include <stdint.h>


struct hash_bucket {
	uint64_t hash;
	char* key;
	void* value;
};


typedef struct hash_table {
	size_t alloc_size;
	size_t fill;
	float grow_ratio; // default 0.75
	float shrink_ratio; // set greater than 1.0 to entirely disable, default 99.0
	struct hash_bucket* buckets; 
} HashTable;

#define HashTable(x) struct hash_table

// NOTE: if you pass in garbage pointers you deserve the segfault

HashTable* HT_create(int allocPOT);
int HT_init(HashTable* obj, int allocPOT);
void HT_destroy(HashTable* obj, int free_values_too);
int HT_resize(HashTable* obj, int newSize);

// returns 0 if val is set to the value
// *val == NULL && return > 0  means the key was not found;
int HT_get(HashTable* obj, char* key, void** val);

// zero for success
int HT_set(HashTable* obj, char* key, void* val);

int HT_delete(HashTable* obj, char* key);

// iteration. no order. results undefined if modified while iterating
// returns 0 when there is none left
// set iter to NULL to start
int HT_next(HashTable* obj, void** iter, char** key, void** value);



/*
// special faster version for storing just integer sets
struct int_hash_bucket {
	uint64_t key;
	uint64_t value;
};

typedef struct hash_table {
	size_t alloc_size;
	size_t fill;
	float grow_ratio;
	float shrink_ratio;
	struct int_hash_bucket* buckets; 
} IntHash;

*/







#endif //__EACSMB_HASH_H__
