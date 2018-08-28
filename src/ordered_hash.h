#ifndef __EACSMB_ordered_hash_h__
#define __EACSMB_ordered_hash_h__

#include <stdint.h>


struct ordered_hash_bucket {
	uint64_t hash;
	char* key;
	void* value;
	uint32_t next, prev; // only supports table sizes up to 2^32 buckets
};


typedef struct ordered_hash_table {
	size_t alloc_size;
	size_t fill;
	float grow_ratio; // default 0.75
	float shrink_ratio; // set greater than 1.0 to entirely disable, default 99.0
	struct ordered_hash_bucket* buckets;
	uint32_t first, last;
} OHashTable;

#define OHashTable(x) struct ordered_hash_table

// NOTE: if you pass in garbage pointers you deserve the segfault

OHashTable* OHT_create(int allocPOT);
int OHT_init(OHashTable* obj, int allocPOT);
void OHT_destroy(OHashTable* obj, int free_values_too);
int OHT_resize(OHashTable* obj, int newSize);

// returns 0 if val is set to the value
// *val == NULL && return > 0  means the key was not found;
int OHT_get(OHashTable* obj, char* key, void** val);
int OHT_getInt(OHashTable* obj, char* key, int64_t* val);

// zero for success
// key's memory is not managed internally. strdup it yourself
int OHT_set(OHashTable* obj, char* key, void* val);
int OHT_setInt(OHashTable* obj, char* key, int64_t val);

int OHT_delete(OHashTable* obj, char* key);

// iteration. no order. results undefined if modified while iterating
// returns 0 when there is none left
// set iter to NULL to start
int OHT_next(OHashTable* obj, void** iter, char** key, void** value);

int OHT_prev(OHashTable* obj, void** iter, char** key, void** value);

int OHT_first(OHashTable* obj, void** iter, char** key, void** value);
int OHT_last(OHashTable* obj, void** iter, char** key, void** value);
int OHT_nth(OHashTable* obj, uint32_t n, void** iter, char** key, void** value);

/*
Loop macro magic

https://www.chiark.greenend.org.uk/~sgtatham/mp/

HashTable obj;
HT_LOOP(&obj, key, char*, val) {
	printf("loop: %s, %s", key, val);
}

effective source:

	#define HT_LOOP(obj, keyname, valtype, valname)
	if(0)
		finished: ;
	else
		for(char* keyname;;) // internal declarations, multiple loops to avoid comma op funny business
		for(valtype valname;;)
		for(void* iter = NULL ;;)
			if(HT_next(obj, iter, &keyname, &valname))
				goto main_loop;
			else
				while(1)
					if(1) {
						// when the user uses break
						goto finished;
					}
					else
						while(1)
							if(!HT_next(obj, iter, &keyname, &valname)) {
								// normal termination
								goto finished;
							}
							else
								main_loop:
								//	{ user block; not in macro }
*/
#define OHASH__PASTEINNER(a, b) a ## b
#define OHASH__PASTE(a, b) OHASH__PASTEINNER(a, b) 
#define OHASH__ITER(key, val) OHASH__PASTE(hashtable_iter_ ## key ## __ ## val ## __, __LINE__)
#define OHASH__FINISHED(key, val) OHASH__PASTE(hashtable_finished__ ## key ## __ ## val ## __, __LINE__)
#define OHASH__MAINLOOP(key, val) OHASH__PASTE(hashtable_main_loop__ ## key ## __ ## val ## __, __LINE__)    
#define HT_LOOP(obj, keyname, valtype, valname) \
if(0) \
	OHASH__FINISHED(key, val): ; \
else \
	for(char* keyname ;;) \
	for(valtype valname ;;) \
	for(void* OHASH__ITER(key, val) = NULL ;;) \
		if(HT_next(obj, & (OHASH__ITER(key, val)), &keyname, (void**)&valname)) \
			goto OHASH__MAINLOOP(key, val); \
		else \
			while(1) \
				if(1) { \
					goto OHASH__FINISHED(key, val); \
				} \
				else \
					while(1) \
						if(!HT_next(obj, & (OHASH__ITER(key, val)), &keyname, (void**)&valname)) { \
							goto OHASH__FINISHED(key, val); \
						} \
						else \
							OHASH__MAINLOOP(key, val) :
							
							//	{ user block; not in macro }









#endif //__EACSMB_ORDERED_HASH_H__
