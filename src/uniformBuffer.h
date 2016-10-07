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


void initUniformBuffers();

void uniformBuffer_init(UniformBuffer* ub, size_t region_size);
void uniformBuffer_bindProg(UniformBuffer* ub, GLuint prog_id, char* name);

void* uniformBuffer_begin(UniformBuffer* ub);
void uniformBuffer_bindRange(UniformBuffer* ub);
void uniformBuffer_finish(UniformBuffer* ub);

#endif // __EACSMB_UNIFORMBUFFER_H__