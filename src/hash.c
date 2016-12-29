 

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "MurmurHash3.h"
#include "hash.h"



#define MURMUR_SEED 718281828

 
static uint64_t hash_key(char* key, size_t len);
static size_t find_bucket(HashTable* obj, uint64_t hash, char* key);
 
 
HashTable* HT_create(int allocPOT) {
	
	int pot, allocSz;
	HashTable* obj;
	
	pot = allocPOT < 16 ? 16 : allocPOT;
	
	obj = malloc(sizeof(*obj));
	if(!obj) return NULL;
	
	obj->fill = 0;
	obj->alloc_size = 1 << pot;
	obj->grow_ratio = 0.75f;
	obj->shrink_ratio = 99.0f;
	obj->buckets = calloc(1, sizeof(*obj->buckets) * obj->alloc_size);
	if(!obj->buckets) {
		free(obj);
		return NULL;
	}
	
	return obj;
}




// uses a truncated 128bit murmur3 hash
static uint64_t hash_key(char* key, size_t len) {
	uint64_t hash[2];
	
	// len is optional
	if(len == -1) len = strlen(key);
	
	MurmurHash3_x64_128(key, len, MURMUR_SEED, hash);
	
	return hash[0];
}

static size_t find_bucket(HashTable* obj, uint64_t hash, char* key) {
	size_t startBucket, bi;
	
	bi = startBucket = hash % obj->alloc_size; 
	
	do {
		struct hash_bucket* bucket;
		
		bucket = &obj->buckets[bi];
		
		// empty bucket
		if(bucket->key == NULL) {
			return bi;
		}
		
		if(bucket->hash == hash) {
			if(!strcmp(key, bucket->key)) {
				// bucket is the right one and contains a value already
				return bi;
			}
			
			// collision, probe next bucket
		}
		
		bi = (bi + 1) % obj->alloc_size;
	} while(bi != startBucket);
	
	// should never reach here if the table is maintained properly
	return -1;
}






// should always be called with a power of two
int HT_resize(HashTable* obj, int newSize) {
	struct hash_bucket* old, *op;
	size_t oldlen = obj->alloc_size;
	size_t i, n, bi;
	
	old = op = obj->buckets;
	
	obj->alloc_size = newSize;
	obj->buckets = calloc(1, sizeof(*obj->buckets) * newSize);
	if(!obj->buckets) return 1;
	
	for(i = 0, n = 0; i < oldlen && n < obj->fill; i++) {
		if(op->key == NULL) continue;
		
		bi = find_bucket(obj, op->hash, op->key);
		obj->buckets[bi].value = op->value;
		obj->buckets[bi].hash = op->hash;
		obj->buckets[bi].key = op->key;
		
		n++;
	}
	
	free(old);
	
	return 0;
}

// TODO: better return values and missing key handling
// returns 0 if val is set to the value
// *val == NULL && return 0  means the key was not found;
int HT_get(HashTable* obj, char* key, void** val) {
	uint64_t hash;
	size_t bi;
	
	hash = hash_key(key, -1);
	
	bi = find_bucket(obj, hash, key);
	if(bi < 0) return 1;
	
	*val = obj->buckets[bi].value; 
	return 0;
}

// zero for success
int HT_set(HashTable* obj, char* key, void* val) {
	uint64_t hash;
	size_t bi;
	
	// TODO: check size and grow if necessary
	
	hash = hash_key(key, -1);
	
	bi = find_bucket(obj, hash, key);
	if(bi < 0) return 1;
	
	obj->buckets[bi].value = val;
	
	return 0;
}


