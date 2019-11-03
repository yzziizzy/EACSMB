#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"

#include "utilities.h"

#include "uniformBuffer.h"


static GLuint ubo_alignment;



// terrible code, but use for now
static int waitSync(GLsync id) {
	GLenum ret;
	if(!id || !glIsSync(id)) return 1;
	while(1) {
		ret = glClientWaitSync(id, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		glexit("");
		if(ret == GL_ALREADY_SIGNALED || ret == GL_CONDITION_SATISFIED)
			return 0;
	}
}


void* uniformBuffer_begin(UniformBuffer* ub) {
	// the fence at index n protects from writing to index n.
	// it is set after commands for n - 1;
	waitSync(ub->fences[ub->next_region]);
	
	return ub->data_ptr + (ub->next_region * ubo_alignment);
}

void uniformBuffer_bindRange(UniformBuffer* ub) {
	glBindBuffer(GL_UNIFORM_BUFFER, ub->ubo);
	glBindBufferRange(
		GL_UNIFORM_BUFFER, 
		0, 
		ub->ubo, 
		ub->next_region * ub->region_size, 
		ub->region_size);
	glexit("");
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glexit("");
}



void uniformBuffer_finish(UniformBuffer* ub) {
	
	if(ub->fences[ub->next_region]) glDeleteSync(ub->fences[ub->next_region]);
	glexit("");
	ub->fences[ub->next_region] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glexit("");
	
	ub->next_region = (ub->next_region + 1) % UNIFORM_BUFFER_DEPTH; // BUG: make sure this is the right one
	
}



void initUniformBuffers() {
	
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &ubo_alignment);
	glexit("");
}


void uniformBuffer_bindProg(UniformBuffer* ub, GLuint prog_id, char* name) {
	GLuint block_index;
	
	glBindBuffer(GL_UNIFORM_BUFFER, ub->ubo);
	
	block_index = glGetUniformBlockIndex(prog_id, name);
	glexit("");
	glUniformBlockBinding(prog_id, block_index, 0);
	glexit("");
	
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glexit("");
}


void uniformBuffer_init(UniformBuffer* ub, size_t region_size) {
	
	GLbitfield flags;
	size_t ubo_size;
	
	// make sure it's aligned
	ub->region_size = ((region_size / ubo_alignment) * ubo_alignment) + (region_size % ubo_alignment == 0 ? 0 : ubo_alignment);
	
	ub->next_region = 0;
	memset(ub->fences, 0, sizeof(ub->fences));
	
	flags =  GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	ubo_size = ub->region_size * UNIFORM_BUFFER_DEPTH;
	
	glGenBuffers(1, &ub->ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ub->ubo);
	glBufferStorage(GL_UNIFORM_BUFFER, ubo_size, NULL, flags);
	glexit("ubo storage");
	
	ub->data_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, ubo_size, flags);
	glexit("ubo persistent map");
	
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glexit("");
}
