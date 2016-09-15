 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"

#include "renderer.h"




static void render_DrawArraysInstanced(RC_DrawArraysInstanced* cb) {
	
	glBindVertexArray(rc->vao);
	glBindBuffer(GL_ARRAY_BUFFER, rc->vbo1);
	if(rc->vbo2) glBindBuffer(GL_ARRAY_BUFFER, rc->vbo2);
	glexit("");
	
	glDrawArraysInstanced(rc->mode, rc->first, rc->count, rc->primcount);
	glexit("glDrawArraysInstanced");
}





void render_RenderCommandQueue(RenderCommand* rc, int length) {
	
	int i;
	RenderCommand* cur, *prev;
	
	prev = NULL;
	cur = rc;
	
	for(i = 0; i < length; i++) {
		
		// shader 
		if(!prev || prev->prog != cur->prog) 
			glUseProgram(cur->prog);
		
		// vertex info
		if(!prev || prev->vao != cur->vao)
			glBindVertexArray(cur->vao);
		
		if(!prev || prev->vbo1 != cur->vbo1)
			glBindVertexArray(cur->vbo1);
		
		if(!prev || prev->vbo2 != cur->vbo2)
			glBindVertexArray(cur->vbo2);
		
		if(!prev || prev->patchVertices != cur->patchVertices)
			glPatchParameteri(GL_PATCH_VERTICES, cur->patchVertices);
		
		
		// depth testing
		if(!prev || prev->depthEnable != cur->depthEnable) {
			if(cur->depthEnable) glEnable(GL_DEPTH_TEST);
			else glDisable(GL_DEPTH_TEST);
		}
		
		if(!prev || prev->depthMask != cur->depthMask)
			glDepthMask(cur->depthMask);
		
		// alpha blending
		if(!prev || prev->blendEnable != cur->blendEnable) {
			if(cur->blendEnable) glEnable(GL_BLEND);
			else glDisable(GL_BLEND);
		}
		
		if(!prev || prev->blendFuncSrc != cur->blendFuncSrc || prev->blendFuncDst != cur->blendFuncDst) {
			glBlendFunc (cur->blendFuncSrc, cur->blendFuncDst);
		}
		
#define CASE_TYPE_CALL(x) case RCT_##x: render_##x(&cur->x); break
		
		switch(cur->type) {
			CASE_TYPE_CALL(DrawArraysInstanced);
			
			
			default:
				fprint(stderr, "Invalid Draw Command: %d\n", cur->type);
		}
		
#undef CASE_TYPE_CALL(x)
		
		prev = cur;
		cur++;
	}

}




RenderCommandQueue* render_AllocCommandQueue(int size) {
	
	RenderCommandQueue* rcq;
	
	rcq = malloc(sizeof(RenderCommandQueue));
	
	rcq->alloc_len = size;
	rcq->next_index = 0;
	rcq->cmds = calloc(1, sizeof(RenderCommand) * size);
	
	return rcq;
}

RenderCommand* render_GetCommandSlot(RenderCommandQueue* rcq) {
	if(rcq->next_index >= rcq->alloc_len) return NULL; 
	return &rcq->cmds[rcq->next_index++];
}





