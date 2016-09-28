#ifndef __EACSMB_UNIFORMBUFFER_H__
#define __EACSMB_UNIFORMBUFFER_H__


// buffer depth. must be at least 3
#define UNIFORM_BUFFER_DEPTH 3

typedef struct UniformBuffer {
	
	GLuint ubo;
	GLuint fences[UNIFORM_BUFFER_DEPTH];
	int next_region;
	
	size_t region_size;
	void* data_ptr;
	
} UniformBuffer;






#endif // __EACSMB_UNIFORMBUFFER_H__