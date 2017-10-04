#ifndef __EACSMB_pcBuffer_h__
#define __EACSMB_pcBuffer_h__

/*************************************\
|                                     |
| Persistent Coherent Mapped Buffers  |
|                                     |
\*************************************/

#include <stdint.h>
#include "common_gl.h"


// shouldn't need to be higher.
#define PC_BUFFER_DEPTH 3


typedef struct PCBuffer {
	size_t bufferSize; // per region. this is the usable space. 
	
	GLsync fences[PC_BUFFER_DEPTH];
	int nextRegion;
	
	GLenum type;
	GLuint bo;
	void* dataPtr;
	
} PCBuffer;


PCBuffer* PCBuffer_alloc(size_t size, GLenum type);
void PCBuffer_startInit(PCBuffer* b, size_t size, GLenum type);
void PCBuffer_finishInit(PCBuffer* b);

void* PCBuffer_beginWrite(PCBuffer* b);
void PCBuffer_finishWrite(PCBuffer* b);
void PCBuffer_bind(PCBuffer* b);




#endif // __EACSMB_pcBuffer_h__
