#ifndef __EACSMB_mempool_h__
#define __EACSMB_mempool_h__

#include <stdlib.h>

// MemPool is a FAST, non-thread-safe allocator for fixed-size objects.
//  There are no system calls during allocation or freeing, though the kernel may run
//  interrupts when a new page of virtual memory is written to for the first time.

// Does not:
//   Handle doulbe free()'s. Only call free once.
//   Handle threads. Manage synchronization yourself.
//   Grow the pool dynamically. Request enough space from the start.
//   Reserve physical memory with the OS. It's purely virtual until you use it.



typedef struct MemPool {
	
	size_t itemSize;
	size_t maxItems;
	
	size_t fill;
	size_t highestUsed;
	
	size_t firstFree;
	
	void* pool;
	
	
} MemPool;

// DO NOT USE THIS:
// these allocate the pool itself
MemPool* MemPool_alloc(size_t itemSize, size_t maxItems);
void MemPool_init(MemPool* mp, size_t itemSize, size_t maxItems);

// USE THIS:
// these allocate chunks of memory from the pool
void* MemPool_malloc(MemPool* mp);
void MemPool_free(MemPool* mp, void* ptr);



#endif //__EACSMB_mempool_h__
