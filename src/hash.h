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
	float grow_ratio;
	float shrink_ratio;
	struct hash_bucket* buckets; 
} HashTable;


HashTable* HT_create(int allocPOT);
int HT_resize(HashTable* obj, int newSize);
int HT_get(HashTable* obj, char* key, void** val);
int HT_set(HashTable* obj, char* key, void* val);

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
