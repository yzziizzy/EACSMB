
#include <string.h>




#include "pass.h"




enum {
	DIFFUSE = 0,
	NORMAL,
	LIGHTING,
	DEPTH,
	OUTPUT,
	DEPTH2
};






void Pass_initPass(Pass* p) {
	
	VEC_INIT(&p->drawables);
	
}




void RenderPipeline_initFBOs(RendererPipeline* bp) {
	
	if(bp->viewSz.x * bp->viewSz.y < 0) {
		printf("render fbo size is less than zero\n");
		return;
	}
	
	// diffuse, normal, lighting, depth, output, NULL
	FBOTexConfig* texcfg = calloc(1, 7 * sizeof(*texcfg)); 
	
	texcfg[DIFFUSE].internalType = GL_RGB;
	texcfg[DIFFUSE].format = GL_RGB;
	texcfg[DIFFUSE].size = GL_UNSIGNED_BYTE;
	texcfg[NORMAL].internalType = GL_RGB;
	texcfg[NORMAL].format = GL_RGB;
	texcfg[NORMAL].size = GL_UNSIGNED_BYTE;
	texcfg[LIGHTING].internalType = GL_RGB16F;
	texcfg[LIGHTING].format = GL_RGB;
	texcfg[LIGHTING].size = GL_HALF_FLOAT;
	texcfg[DEPTH].internalType = GL_DEPTH_COMPONENT32;
	texcfg[DEPTH].format = GL_DEPTH_COMPONENT;
	texcfg[DEPTH].size = GL_FLOAT;
	texcfg[OUTPUT].internalType = GL_RGBA8;
	texcfg[OUTPUT].format = GL_RGBA;
	texcfg[OUTPUT].size = GL_UNSIGNED_BYTE;
	texcfg[DEPTH2].internalType = GL_DEPTH_COMPONENT32;
	texcfg[DEPTH2].format = GL_DEPTH_COMPONENT;
	texcfg[DEPTH2].size = GL_FLOAT;
	
	
	// TODO: clean up old backing textures if existing
	
	bp->backingTextures = initFBOTextures(bp->viewSz.x, bp->viewSz.y, texcfg);
	
	
	FBOConfig gbufConf[] = {
		{GL_COLOR_ATTACHMENT0, bp->backingTextures[DIFFUSE] },
		{GL_COLOR_ATTACHMENT1, bp->backingTextures[NORMAL] },
		{GL_COLOR_ATTACHMENT2, bp->backingTextures[LIGHTING] },
		{GL_DEPTH_ATTACHMENT, bp->backingTextures[DEPTH] },
		{0,0}
	};
	
	initFBO(&bp->fbos[0], gbufConf);
	
	
	FBOConfig sbufConf[] = {
		{GL_COLOR_ATTACHMENT0, bp->backingTextures[OUTPUT] },
		{GL_DEPTH_ATTACHMENT, bp->backingTextures[DEPTH2] },
		{0,0}
	};

	initFBO(&bp->fbos[1], sbufConf);
	
	printf("output tex: %d \n", bp->backingTextures[OUTPUT]);

}



void RenderPass_init(RendererPass* bp, ShaderProgram* prog) {
	
	bp->prog = prog;
	
	// these will generate harmless errors if the uniform is not found
	bp->diffuseUL = glGetUniformLocation(prog->id, "sDiffuse");
	bp->normalsUL = glGetUniformLocation(prog->id, "sNormals");
	bp->lightingUL = glGetUniformLocation(prog->id, "sLighting");
	bp->depthUL = glGetUniformLocation(prog->id, "sDepth");
	glerr("harmless");
}



void RenderPipeline_renderAll(RenderPipeline* bp, PassDrawParams* rp) {
	
	
	for(int i = 0; i < VEC_LEN(&bp->passes); i++) {
		RenderPass* pass = VEC_ITEM(&bp->passes, i);
		ShaderProgram* prog = pass->prog;
		
		glBindFramebuffer(GL_FRAMEBUFFER, bp->fbos[pass->fboIndex].fb);
		glDepthFunc(GL_LEQUAL);
		//printf("fbo bound: %d\n", bp->fbos[pass->fboIndex].fb);
		
		int clear = 0;
		if(pass->clearColor) clear |= GL_COLOR_BUFFER_BIT;
		if(pass->clearDepth) clear |= GL_DEPTH_BUFFER_BIT;
		if(clear) {
			//printf("clearing builder fbo\n");
		//	glClearColor(.8, .1, .3, 1.0);
			glClear(clear);
		}
		
		// TODO: ensure the backin textures are not bound to an active fbo
		if(pass->diffuseUL != -1) { printf("binding diffuse tex\n");
			glActiveTexture(GL_TEXTURE0 + 10);
			glBindTexture(GL_TEXTURE_2D, bp->backingTextures[DIFFUSE]);
			glProgramUniform1i(prog->id, pass->diffuseUL, 10); // broken
		}
		
		if(pass->normalsUL != -1) {
			glActiveTexture(GL_TEXTURE0 + 11);
			glBindTexture(GL_TEXTURE_2D, bp->backingTextures[NORMAL]);
			glProgramUniform1i(prog->id, pass->normalsUL, 11);
		}
		
		if(pass->lightingUL != -1) {
			glActiveTexture(GL_TEXTURE0 + 12);
			glBindTexture(GL_TEXTURE_2D, bp->backingTextures[LIGHTING]);
			glProgramUniform1i(prog->id, pass->lightingUL, 12);
		}
		
		if(pass->depthUL != -1) {
			glActiveTexture(GL_TEXTURE0 + 13);
			glBindTexture(GL_TEXTURE_2D, bp->backingTextures[DEPTH]);
			glProgramUniform1i(prog->id, pass->depthUL, 13);
		}
		
		
		//glDepthFunc(GL_LEQUAL);
// 		glDepthMask(GL_TRUE);
		RenderPass_renderAll(pass, rp);
		
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}



void RenderPass_renderAll(RenderPass* pass, PassDrawParams* rp) {
	
	int i;
	
	
	for(i = 0; i < VEC_LEN(&pass->renderables); i++) {
		PassDrawable* d = &VEC_ITEM(&pass->drawables, i);
		glUseProgram(pass->prog->id);
		d->render(d->data, pass->prog, pass);
	}
	
}


void RenderPipeline_init(RenderPipeline* bp) {
	
	VEC_INIT(&bp->passes);
	
}



PassDrawable* Pass_allocDrawable(char* name) {
	
	PassDrawable* d;
	
	d = calloc(1, sizeof(*d));
	CHECK_OOM(d);
	
	d->name = name;
	
	return d;
}









