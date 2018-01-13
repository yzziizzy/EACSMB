
#include <string.h>




#include "pass.h"




GLuint quadVAO, quadVBO;




enum {
	DIFFUSE = 0,
	NORMAL,
	LIGHTING,
	DEPTH,
	OUTPUT,
	DEPTH2
};





void initRenderPipeline() {
	
	float vertices[] = {
		-1.0, -1.0, 0.0,
		-1.0, 1.0, 0.0,
		1.0, -1.0, 0.0,
		1.0, 1.0, 0.0
	};
	
	glGenVertexArrays(1, &quadVAO);
	glBindVertexArray(quadVAO);
	
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
}




static void shading_pass_render(RenderPipeline* rpipe, PassDrawable* pd, PassDrawParams* pdp) {
	ShaderProgram* prog = pd->prog;
	
//	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "world"), 1, GL_FALSE, world.m);
	glUniformMatrix4fv(glGetUniformLocation(prog->id, "mViewProj"), 1, GL_FALSE, pdp->mWorldView->m);
	glUniformMatrix4fv(glGetUniformLocation(prog->id, "mWorldView"), 1, GL_FALSE, pdp->mViewProj->m);
	glexit("");
	
// 	mInverse(msGetTop(&gs->proj), &projView);
// 	mInverse(msGetTop(&gs->view), &viewWorld);
	
	glUniformMatrix4fv(glGetUniformLocation(prog->id, "mProjView"), 1, GL_FALSE, pdp->mProjView->m);
	glUniformMatrix4fv(glGetUniformLocation(prog->id, "mViewWorld"), 1, GL_FALSE, pdp->mViewWorld->m);
	glexit("");
	
// 	glUniform3fv(glGetUniformLocation(shadingProg->id, "sunNormal"), 1, (float*)&gs->sunNormal);
	
// 	glUniform2iv(glGetUniformLocation(prog->id, "resolution"), 1, (int*)&rp->fboSize);
	glUniform2f(glGetUniformLocation(prog->id, "resolution"), rpipe->viewSz.x, rpipe->viewSz.x);
	glexit("");
	
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glexit("quad vbo");

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("quad draw");
} 


void RenderPipeline_addShadingPass(RenderPipeline* rpipe, char* shaderName) {
	RenderPass* pass;
	
	// shading pass
	pass = calloc(1, sizeof(*pass));
	pass->fboIndex = 1;
	pass->clearColor = 1;
	pass->clearDepth = 1;
	
	
	RenderPass_init(pass, loadCombinedProgram(shaderName));
	
	PassDrawable* d = calloc(1, sizeof(*d));
	d->draw = shading_pass_render;
	d->prog = pass->prog;
	d->data = rpipe;
	
	VEC_PUSH(&pass->drawables, d);
	
	
	VEC_PUSH(&rpipe->passes, pass);
	
	
}










void RenderPipeline_rebuildFBOs(RenderPipeline* rp, Vector2i sz) {
	
	if(rp->viewSz.x == sz.x && rp->viewSz.x == sz.x && rp->backingTextures) {
		// same size, already initialized.
		printf("ignoring pipeline resize, 1\n");
		return;
	}
	
	if(sz.x * sz.y < 1) {
		printf("render fbo size is less than zero\n");
		return;
	}
	
	rp->viewSz = sz;
	
	if(rp->backingTextures) {
		destroyFBOTextures(rp->backingTextures);
		free(rp->backingTextures);
		
		destroyFBO(&rp->fbos[0]);
		destroyFBO(&rp->fbos[1]);
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
	
	rp->backingTextures = initFBOTextures(rp->viewSz.x, rp->viewSz.y, texcfg);
	
	
	FBOConfig gbufConf[] = {
		{GL_COLOR_ATTACHMENT0, rp->backingTextures[DIFFUSE] },
		{GL_COLOR_ATTACHMENT1, rp->backingTextures[NORMAL] },
		{GL_COLOR_ATTACHMENT2, rp->backingTextures[LIGHTING] },
		{GL_DEPTH_ATTACHMENT, rp->backingTextures[DEPTH] },
		{0,0}
	};
	
	initFBO(&rp->fbos[0], gbufConf);
	
	
	FBOConfig sbufConf[] = {
		{GL_COLOR_ATTACHMENT0, rp->backingTextures[OUTPUT] },
		{GL_DEPTH_ATTACHMENT, rp->backingTextures[DEPTH2] },
		{0,0}
	};

	initFBO(&rp->fbos[1], sbufConf);
	
	printf("output tex: %d \n", rp->backingTextures[OUTPUT]);

}



void RenderPass_init(RenderPass* bp, ShaderProgram* prog) {
	
	bp->prog = prog;
	
	// these will generate harmless errors if the uniform is not found
	bp->diffuseUL = glGetUniformLocation(prog->id, "sDiffuse");
	bp->normalsUL = glGetUniformLocation(prog->id, "sNormals");
	bp->lightingUL = glGetUniformLocation(prog->id, "sLighting");
	bp->depthUL = glGetUniformLocation(prog->id, "sDepth");
	glerr("harmless");
}


void RenderPipeline_destroy(RenderPipeline* rp) {
	
	if(rp->backingTextures) {
		destroyFBOTextures(rp->backingTextures);
		free(rp->backingTextures);
		
		destroyFBO(&rp->fbos[0]);
		destroyFBO(&rp->fbos[1]);
	}
	
	// TODO: clean up all the children of the pass
}


void RenderPipeline_renderAll(RenderPipeline* bp, PassDrawParams* rp) {
	
	if(!bp->backingTextures) return;
	
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



void RenderPass_renderAll(RenderPass* pass, PassDrawParams* pdp) {
	
	int i;
	
	
	for(i = 0; i < VEC_LEN(&pass->drawables); i++) {
		PassDrawable* d = VEC_ITEM(&pass->drawables, i);
		glUseProgram(pass->prog->id);
		d->draw(d->data, d, pdp);
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





GLuint RenderPipeline_getOutputTexture(RenderPipeline* rp) {
	if(!rp->backingTextures) return 0;
	return rp->backingTextures[OUTPUT];
}



