
#include "ds.h"





void inline vec_resize(void** data, size_t* size, size_t elem_size) {
	void* tmp;
	
	if(*size < 8) *size = 8;
	else *size *= 2;
	
	tmp = realloc(*data, *size * elem_size);
	if(!tmp) {
		fprintf(stderr, "Out of memory in vector resize");
		return;
	}
	
	*data = tmp;
}
 
ptrdiff_t inline vec_find(void* data, size_t len, size_t stride, void* search) {
	size_t i;
	for(i = 0; i < len; i++) {
		if(memcmp(data + (i * stride), search, stride)) {
			return i;
		}
	}
	
	return -1;
}