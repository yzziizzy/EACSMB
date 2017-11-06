#ifndef __DS_H__
#define __DS_H__

#include <stdint.h>
#include <malloc.h>



// declare a vector
#define VEC(t) \
struct { \
	size_t len, alloc; \
	t* data; \
}

// initialisze a vector
#define VEC_INIT(x) \
do { \
	(x)->data = NULL; \
	(x)->len = 0; \
	(x)->alloc = 0; \
} while(0)


// helpers
#define VEC_LEN(x) ((x)->len)
#define VEC_ALLOC(x) ((x)->alloc)
#define VEC_DATA(x) ((x)->data)
#define VEC_ITEM(x, i) (VEC_DATA(x)[(i)])

#define VEC_TAIL(x) (VEC_DATA(x)[VEC_LEN(x)-1])
#define VEC_HEAD(x) (VEC_DATA(x)[0])

#define VEC_FIND(x, ptr_o) vec_find(VEC_DATA(x), VEC_LEN(x), sizeof(*VEC_DATA(x)), ptr_o)

//  

#define VEC_GROW(x) vec_resize((void**)&VEC_DATA(x), &VEC_ALLOC(x), sizeof(*VEC_DATA(x)))

// check if a size increase is needed to insert one more item
#define VEC_CHECK(x) \
do { \
	if(VEC_LEN(x) >= VEC_ALLOC(x)) { \
		VEC_GROW(x); \
	} \
} while(0)


// operations

// increase size and assign the new entry
#define VEC_PUSH(x, e) \
do { \
	VEC_CHECK(x); \
	VEC_DATA(x)[VEC_LEN(x)] = (e); \
	VEC_LEN(x)++; \
} while(0)

// increase size but don't assign
#define VEC_INC(x) \
do { \
	VEC_CHECK(x); \
	VEC_LEN(x)++; \
} while(0)



#define VEC_PEEK(x) VEC_DATA(x)[VEC_LEN(x) - 1]

#define VEC_POP(x, e) \
do { \
	VEC_CHECK(x); \
	(e) = VEC_PEEK(x); \
	VEC_LEN(x)--; \
} while(0)


#define VEC_FREE(x) \
do { \
	if(VEC_DATA(x)) free(VEC_DATA(x)); \
	VEC_DATA(x) = NULL; \
	VEC_LEN(x) = 0; \
	VEC_ALLOC(x) = 0; \
} while(0)

#define VEC_COPY(copy, orig) \
do { \
	void* tmp; \
	tmp = realloc(VEC_DATA(copy), VEC_ALLOC(orig) * sizeof(*VEC_DATA(orig)) ); \
	if(!tmp) { \
		fprintf(stderr, "Out of memory in vector copy"); \
		return; \
	} \
	\
	VEC_DATA(copy) = tmp; \
	VEC_LEN(copy) = VEC_LEN(orig); \
	VEC_ALLOC(copy) = VEC_ALLOC(orig); \
	\
	memcpy(VEC_DATA(copy), VEC_DATA(orig),  VEC_LEN(orig) * sizeof(*VEC_DATA(orig))); \
} while(0)


#define VEC_REVERSE(x) \
do { \
	size_t i, j; \
	void* tmp = alloca(sizeof(*VEC_DATA(x))); \
	for(i = 0, j = VEC_LEN(x); i < j; i++, j--) { \
		memcpy(tmp, VEC_DATA(x)[i]); \
		memcpy(VEC_DATA(x)[i], VEC_DATA(x)[j]); \
		memcpy(VEC_DATA(x)[j], tmp); \
	} \
} while(0)


#define VEC_SPLICE(x, y, where) \
do { \
	if(VEC_ALLOC(x) < VEC_LEN(x) + VEC_LEN(y)) { \
		vec_resize_to((void**)&VEC_DATA(x), &VEC_ALLOC(x), sizeof(*VEC_DATA(x)), VEC_LEN(x) + VEC_LEN(y)); \
	} \
	\
	memcpy( /* move the rest of x forward */ \
		VEC_DATA(x) + where + VEC_LEN(y), \
		VEC_DATA(x) + where,  \
		(VEC_LEN(x) - where) * sizeof(*VEC_DATA(x)) \
	); \
	memcpy( /* copy y into the space created */ \
		VEC_DATA(x) + where, \
		VEC_DATA(y),  \
		VEC_LEN(y) * sizeof(*VEC_DATA(y)) \
	); \
} while(0)
	


void vec_resize(void** data, size_t* size, size_t elem_size);
ptrdiff_t vec_find(void* data, size_t len, size_t stride, void* search);









#endif // __DS_H__
