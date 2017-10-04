
#include <stdlib.h>
#include <string.h>

#include "utilities.h"
#include "pcBuffer.h"



static int waitSync(GLuint id); 


PCBuffer* PCBuffer_alloc(size_t size, GLenum type) {
	PCBuffer* b;
	
	b = malloc(sizeof(*b));
	CHECK_OOM(b);
	
	PCBuffer_startInit(b, size, type);
	
	return b;
}


void PCBuffer_startInit(PCBuffer* b, size_t size, GLenum type) {
	GLbitfield flags;

	flags =  GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	
	b->type = type;
	b->bufferSize = size;
	
	memset(b->fences, 0, sizeof(b->fences));
	b->nextRegion= 0;
	
	
	glGenBuffers(1, &b->bo);
	glexit("PCBuffer GenBuffers");
	
	glBindBuffer(b->type, b->bo);
	glBufferStorage(b->type, size * PC_BUFFER_DEPTH, NULL, flags);
	glexit("PCBuffer storage alloc");
	
	// the buffer is left bound
	// do any buffer config then call  PCBuffer_finishInit()
}



void PCBuffer_finishInit(PCBuffer* b) {
	GLbitfield flags;

	flags =  GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	
	b->dataPtr = glMapBufferRange(b->type, 0, b->bufferSize * PC_BUFFER_DEPTH, flags);
	glexit("PCBuffer persistent map");
	
	glBindBuffer(b->type, 0);
}


void* PCBuffer_beginWrite(PCBuffer* b) {
	// the fence at index n protects from writing to index n.
	// it is set after commands for n - 1;
	waitSync(b->fences[b->nextRegion]);
	
	return &b->dataPtr[b->nextRegion * b->bufferSize];
}


void PCBuffer_afterDraw(PCBuffer* b) {
	
	if(b->fences[b->nextRegion]) glDeleteSync(b->fences[b->nextRegion]);
	glexit("");
	
	b->fences[b->nextRegion] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glexit("");
	
	b->nextRegion = (b->nextRegion + 1) % PC_BUFFER_DEPTH; 
}



void PCBuffer_bind(PCBuffer* b) {
	glBindBuffer(b->type, b->bo);
}


// terrible code, but use for now
static int waitSync(GLuint id) {
	GLenum ret;
	if(!id || !glIsSync(id)) return 1;
	while(1) {
		ret = glClientWaitSync(id, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		glexit("");
		if(ret == GL_ALREADY_SIGNALED || ret == GL_CONDITION_SATISFIED)
			return 0;
	}
}



