 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "MurmurHash3.h"
#include "ordered_hash.h"



#define MURMUR_SEED 718281828

 
static uint64_t hash_key(char* key, size_t len);
static int64_t find_bucket(OHashTable* obj, uint64_t hash, char* key);
 
 
OHashTable* OHT_create(int allocPOT) {
	
	OHashTable* obj;
	
	obj = malloc(sizeof(*obj));
	if(!obj) return NULL;
	
	if(OHT_init(obj, allocPOT)) {
		fprintf(stderr, "Failed to initialized hash table\n");
		free(obj);
		return NULL;
	}
	
	return obj;
}


int OHT_init(OHashTable* obj, int allocPOT) {
	int pot, allocSz;
	
	pot = allocPOT < 4 ? 4 : allocPOT;
	
	obj->fill = 0;
	obj->alloc_size = 1 << pot;
	obj->grow_ratio = 0.75f;
	obj->shrink_ratio = 99.0f; // set greater than 1.0 to entirely disable
	obj->buckets = calloc(1, sizeof(*obj->buckets) * obj->alloc_size);
	if(!obj->buckets) {
		return 1;
	}
	
	return 0;
}


void OHT_destroy(OHashTable* obj, int free_values_too) {
	int i, n;
	
	if(free_values_too) {
		for(i = 0, n = 0; i < obj->alloc_size, n < obj->fill; i++) {
			// only free valid pointers that also have a key
			// deleted items are assumed to be cleaned up by the user
			if(obj->buckets[i].key) {
				if(obj->buckets[i].value) free(obj->buckets[i].value);
				n++;
			}
		}
	}
	
	if(obj->buckets) free(obj->buckets);
	free(obj);
}



// uses a truncated 128bit murmur3 hash
static uint64_t hash_key(char* key, size_t len) {
	uint64_t hash[2];
	
	// len is optional
	if(len == -1) len = strlen(key);
	
	MurmurHash3_x64_128(key, len, MURMUR_SEED, hash);
	
	return hash[0];
}

static int64_t find_bucket(OHashTable* obj, uint64_t hash, char* key) {
	int64_t startBucket, bi;
	
	bi = startBucket = hash % obj->alloc_size; 
	
	do {
		struct ordered_hash_bucket* bucket;
		
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
int OHT_resize(OHashTable* obj, int newSize) {
	struct ordered_hash_bucket* old, *op;
	size_t oldlen = obj->alloc_size;
	int64_t i, n, bi;
	
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
		
		// TODO: fix ordering
		
		
		n++;
	}
	
	free(old);
	
	return 0;
}

// TODO: better return values and missing key handling
// returns 0 if val is set to the value
// *val == NULL && return > 0  means the key was not found;
int OHT_get(OHashTable* obj, char* key, void** val) {
	uint64_t hash;
	int64_t bi;
	
	hash = hash_key(key, -1);
	
	bi = find_bucket(obj, hash, key);
	if(bi < 0 || obj->buckets[bi].key == NULL) {
		*val = NULL;
		return 1;
	}
	
	*val = obj->buckets[bi].value; 
	return 0;
}

int OHT_getInt(OHashTable* obj, char* key, int64_t* val) {
	return OHT_get(obj, key, (void**)val);
} 


// zero for success. added to the end of the list.
int OHT_set(OHashTable* obj, char* key, void* val) {
	uint64_t hash;
	int64_t bi;
	
	// check size and grow if necessary
	if(obj->fill / obj->alloc_size >= obj->grow_ratio) {
		OHT_resize(obj, obj->alloc_size * 2);
	}
	
	hash = hash_key(key, -1);
	
	bi = find_bucket(obj, hash, key);
	if(bi < 0) return 1;
	
	if(obj->buckets[bi].key == NULL) {
		// new bucket
		obj->buckets[bi].key = key;
		obj->buckets[bi].hash = hash;
		obj->fill++;
	}
	
	obj->buckets[bi].value = val;
	obj->buckets[bi].key = key;
	obj->buckets[bi].hash = hash;
	obj->buckets[bi].next = 0;
	obj->buckets[bi].prev = obj->tail;
	obj->tail = bi;
		
	if(obj->fill == 1) {
		obj->head = bi;
	}
	
	return 0;
}
// zero for success
int OHT_setInt(OHashTable* obj, char* key, int64_t val) {
	return OHT_set(obj, key, (void*)val);
}

// zero for success
int OHT_delete(OHashTable* obj, char* key) {
	uint64_t hash;
	int64_t bi, empty_bi, nat_bi;
	
	size_t alloc_size = obj->alloc_size;
	
	/* do this instead of the deletion algorithm
	// check size and shrink if necessary
	if(obj->fill / alloc_size <= obj->shrink_ratio) {
		OHT_resize(obj, alloc_size > 32 ? alloc_size / 2 : 16);
		alloc_size = obj->alloc_size;
	}
	*/
	hash = hash_key(key, -1);
	bi = find_bucket(obj, hash, key);
	
	// if there's a key, work until an empty bucket is found
	// check successive buckets for colliding keys
	//   walk forward until the furthest colliding key is found
	//   move it to the working bucket.
	//   
	
	// nothing to delete, bail early
	if(obj->buckets[bi].key == NULL) return;
	
	//
	empty_bi = bi;
	
	do {
		bi = (bi + 1) % alloc_size;
		if(obj->buckets[bi].key == NULL) {
			//empty bucket
			break;
		}
		
		// bucket the hash at the current index naturally would be in
		nat_bi = obj->buckets[bi].hash % alloc_size;
		
		if((bi > empty_bi && // after the start
			(nat_bi <= empty_bi /* in a sequence of probed misses */ || nat_bi > bi /* wrapped all the way around */)) 
			||
			(bi < empty_bi && // wrapped around
			(nat_bi <= empty_bi /* in a sequence of probed misses... */ && nat_bi > bi /* ..from before the wrap */))) {
			
			// move this one back
			obj->buckets[empty_bi].key = obj->buckets[bi].key;
			obj->buckets[empty_bi].hash = obj->buckets[bi].hash;
			obj->buckets[empty_bi].value = obj->buckets[bi].value;
			
			// fix the linked list
			if(obj->head == bi) obj->head = empty_bi;
			if(obj->tail == bi) obj->tail = empty_bi;
			
			obj->buckets[empty_bi].next = obj->buckets[bi].next;
			obj->buckets[empty_bi].prev = obj->buckets[bi].prev;
			
			obj->buckets[obj->buckets[bi].prev].next = empty_bi;
			obj->buckets[obj->buckets[bi].next].prev = empty_bi;
			
			
			empty_bi = bi;
		}
	} while(1);
	
	
	// mark the bucket as empty
	obj->buckets[empty_bi].key = NULL;
	
	// fix the list
	obj->buckets[obj->buckets[empty_bi].prev].next = obj->buckets[empty_bi].next;
	obj->buckets[obj->buckets[empty_bi].next].prev = obj->buckets[empty_bi].prev;
	
			
	return 0;
}

// ordered iteration.
// returns 0 when there is none left
// set iter to NULL to start
int OHT_next(OHashTable* obj, void** iter, char** key, void** value) { 
	struct ordered_hash_bucket* b = *iter;
	
	
	if(obj->fill == 0) return 0;
	
	if(b == NULL) {
		b = &obj->buckets[obj->head];
	}
	else if(obj->tail ==  (b - obj->buckets) / sizeof(*b)) { // end of list
		*value = NULL;
		*key = NULL;
		return 0;
	}
	else {
		b = &obj->buckets[b->next];
	}
	
	*key = b->key;
	*value = b->value;
	*iter = b;
	
	return 1;
}



// reverse ordered iteration.
// returns 0 when there is none left
// set iter to NULL to start
int OHT_prev(OHashTable* obj, void** iter, char** key, void** value) {
	struct ordered_hash_bucket* b = *iter;
	
	
	if(obj->fill == 0) return 0;
	
	if(b == NULL) {
		b = &obj->buckets[obj->tail];
	}
	else if(obj->head ==  (b - obj->buckets) / sizeof(*b)) { // end of list
		*value = NULL;
		*key = NULL;
		return 0;
	}
	else {
		b = &obj->buckets[b->prev];
	}
	
	*key = b->key;
	*value = b->value;
	*iter = b;
	
	return 1;
}




// same signature as next/prev except it always selects the first item
int OHT_first(OHashTable* obj, void** iter, char** key, void** value) {
	struct ordered_hash_bucket* b = *iter;
	
	
	if(obj->fill == 0) return 0;
	
	b = &obj->buckets[obj->head];
	
	*key = b->key;
	*value = b->value;
	*iter = b;
	
	return 1;
}


// same signature as next/prev except it always selects the last item
int OHT_last(OHashTable* obj, void** iter, char** key, void** value) {
	struct ordered_hash_bucket* b = *iter;
	
	
	if(obj->fill == 0) return 0;
	
	b = &obj->buckets[obj->tail];
	
	*key = b->key;
	*value = b->value;
	*iter = b;
	
	return 1;
}

// same general operation as next/prev except it always selects the nth item
int OHT_nth(OHashTable* obj, uint32_t n, void** iter, char** key, void** value) {
	struct ordered_hash_bucket* b = *iter;
	
	
	if(obj->fill == 0) return 0;
	
	
	b = &obj->buckets[obj->head];
	
	while(n--) {
		b = &obj->buckets[b->prev];
	}
	
	*key = b->key;
	*value = b->value;
	*iter = b;
	
	return 1;
}
