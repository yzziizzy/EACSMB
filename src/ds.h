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

#define VEC_TAIL(x) (VEC_DATA(x)[VEC_LEN(x)])
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






void inline vec_resize(void** data, size_t* size, size_t elem_size);
ptrdiff_t inline vec_find(void* data, size_t len, size_t stride, void* search);









#endif // __DS_H__