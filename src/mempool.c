#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/mman.h>

#include "mempool.h"









MemPool* MemPool_alloc(size_t itemSize, size_t maxItems) {
	MemPool* mp;
	
	mp = calloc(1, sizeof(*mp));
	
	MemPool_init(mp, itemSize, maxItems);
	
	return mp;
}



void MemPool_init(MemPool* mp, size_t itemSize, size_t maxItems) {
	size_t allocSize;
	
	mp->itemSize = itemSize < sizeof(size_t) ? sizeof(size_t) : itemSize;
	mp->maxItems = maxItems;
	
	mp->fill = 0;
	mp->firstFree = 1; // first free is 1-based
	
	allocSize = mp->itemSize * mp->maxItems;
	int flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE;
	mp->pool = mmap(NULL, allocSize, PROT_READ | PROT_WRITE, flags, 0, 0);
	if(mp->pool == MAP_FAILED) {
		fprintf(stderr, "mmap failed in %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
}



void* MemPool_malloc(MemPool* mp) {
	if(mp->fill >= mp->maxItems) {
		fprintf(stderr, "MemPool overflowed max items %d\n", mp->maxItems);
		return NULL;
	}
	
	size_t off = mp->itemSize * (mp->firstFree - 1);
	size_t next = *(size_t*)(mp->pool + off);
	if(next == 0) next = mp->firstFree + 1;
	mp->firstFree = next;
	
	// not used in operation
	mp->fill++;
	
	return mp->pool + off;
}


void MemPool_free(MemPool* mp, void* ptr) {
	
	size_t ooff = mp->itemSize * (mp->firstFree - 1);
	size_t noff = (ptr - mp->pool) / mp->itemSize;
	
	*(size_t*)(mp->pool + ooff) = noff + 1;
	
	// not used in operation
	mp->fill--;
}

