



#include "render.h" 






#include "shader.h"



GLuint quadVAO, quadVBO;





void init_Builder() {
	
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




void shading_pass_render(void* data, ShaderProgram* prog, RenderParams* rp) {
//	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "world"), 1, GL_FALSE, world.m);
	glUniformMatrix4fv(glGetUniformLocation(prog->id, "mViewProj"), 1, GL_FALSE, rp->mWorldView->m);
	glUniformMatrix4fv(glGetUniformLocation(prog->id, "mWorldView"), 1, GL_FALSE, rp->mViewProj->m);
	glexit("");
	
// 	mInverse(msGetTop(&gs->proj), &projView);
// 	mInverse(msGetTop(&gs->view), &viewWorld);
	
	glUniformMatrix4fv(glGetUniformLocation(prog->id, "mProjView"), 1, GL_FALSE, rp->mProjView->m);
	glUniformMatrix4fv(glGetUniformLocation(prog->id, "mViewWorld"), 1, GL_FALSE, rp->mViewWorld->m);
	glexit("");
	
// 	glUniform3fv(glGetUniformLocation(shadingProg->id, "sunNormal"), 1, (float*)&gs->sunNormal);
	
// 	glUniform2iv(glGetUniformLocation(prog->id, "resolution"), 1, (int*)&rp->fboSize);
	glUniform2f(glGetUniformLocation(prog->id, "resolution"), 300, 300);
	glexit("");
	
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glexit("quad vbo");

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("quad draw");
} 




enum {
	DIFFUSE = 0,
	NORMAL,
	LIGHTING,
	DEPTH,
	OUTPUT,
	DEPTH2
};





void Builder_initFBOs(BuilderPipeline* bp) {
	
	if(bp->viewSz.x * bp->viewSz.y < 0) {
		printf("builder fbo size is less than zero\n");
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



void BuilderPass_init(BuilderPass* bp, ShaderProgram* prog) {
	
	bp->prog = prog;
	
	// these will generate harmless errors if the uniform is not found
	bp->diffuseUL = glGetUniformLocation(prog->id, "sDiffuse");
	bp->normalsUL = glGetUniformLocation(prog->id, "sNormals");
	bp->lightingUL = glGetUniformLocation(prog->id, "sLighting");
	bp->depthUL = glGetUniformLocation(prog->id, "sDepth");
	glerr("harmless");
}



void BuilderPipeline_renderAll(BuilderPipeline* bp, RenderParams* rp) {
	
	
	for(int i = 0; i < VEC_LEN(&bp->passes); i++) {
		BuilderPass* pass = VEC_ITEM(&bp->passes, i);
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
		BuilderPass_renderAll(pass, rp);
		
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}



void BuilderPass_renderAll(BuilderPass* bp, RenderParams* rp) {
	
	int i;
	
	
	// rebind fbos, textures, etc
	
	for(i = 0; i < VEC_LEN(&bp->renderables); i++) {
		BuilderRenderable* r = &VEC_ITEM(&bp->renderables, i);
		glUseProgram(bp->prog->id);
		r->render(r->data, bp->prog, rp);
	}
	
}


void BuilderPipeline_init(BuilderPipeline* bp) {
	
	VEC_INIT(&bp->passes);
	
}





