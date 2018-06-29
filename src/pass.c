
#include <string.h>




#include "pass.h"




GLuint quadVAO, quadVBO;


struct pass_info {
	pass_callback cb;
	void* data;
	char* name;
};

static HashTable(struct pass_info) prepasses;



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
	
	HT_init(&prepasses, 2);
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
	
	texcfg[DIFFUSE].internalType = GL_RGBA;
	texcfg[DIFFUSE].format = GL_RGBA;
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
			glClearColor(bp->clearColor.x, bp->clearColor.y, bp->clearColor.z, bp->clearColor.w);
			glClear(clear);
		}
		
		// TODO: ensure the backin textures are not bound to an active fbo
		if(pass->diffuseUL != -1) {
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
		
//		 glEnable (GL_BLEND);
		// glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glDepthFunc(GL_LEQUAL);
// 		glDepthMask(GL_TRUE);
		RenderPass_renderAll(pass, rp);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}


int RenderPass_addDrawable(RenderPass* rp, PassDrawable* d) {
	VEC_PUSH(&rp->drawables, d);
}


void RenderPass_renderAll(RenderPass* pass, PassDrawParams* pdp) {
	
	int i;
	
	for(i = 0; i < VEC_LEN(&pass->drawables); i++) {
		PassDrawable* d = VEC_ITEM(&pass->drawables, i);
		
		DrawTimer_Start(&d->timer);
		
		glUseProgram(pass->prog->id);
		d->draw(d->data, d, pdp);
		
		DrawTimer_Start(&d->timer);
	}
}

void RenderPass_postFrameAll(RenderPass* pass) {
	
	int i;
	
	for(i = 0; i < VEC_LEN(&pass->drawables); i++) {
		PassDrawable* d = VEC_ITEM(&pass->drawables, i);
		if(d->postFrame) d->postFrame(d->data);
	}
}
void RenderPass_preFrameAll(RenderPass* pass, PassFrameParams* pfp) {
	
	int i;
	
	for(i = 0; i < VEC_LEN(&pass->drawables); i++) {
		PassDrawable* d = VEC_ITEM(&pass->drawables, i);
		if(d->preFrame) d->preFrame(pfp, d->data);
	}
}


void RenderPipeline_init(RenderPipeline* rp) {
	
	VEC_INIT(&rp->passes);
	
	rp->clearColor = (Vector4){0, 0, 0, 0};
}

void RenderPipeline_Destroy(RenderPipeline* rp) {
	printf("RenderPipeline_Destroy not implemented\n");
}



PassDrawable* Pass_allocDrawable(char* name) {
	
	PassDrawable* d;
	
	d = calloc(1, sizeof(*d));
	CHECK_OOM(d);
	
	d->name = name;
	DrawTimer_Init(&d->timer);
	
	return d;
}





GLuint RenderPipeline_getOutputTexture(RenderPipeline* rp) {
	if(!rp->backingTextures) return 0;
	return rp->backingTextures[OUTPUT];
}










void RegisterPrePass(pass_callback cb, void* data, char* name) {
	
	struct pass_info* pi;
	
	pi = calloc(1, sizeof(*pi));
	pi->data = data;
	pi->name = name;
	pi->cb = cb;
	
	HT_set(&prepasses, name, pi);
}


void RenderAllPrePasses(PassFrameParams* pfp) {
	
	void* iter = NULL;
	char* key;
	struct pass_info* pi;
	
	while(HT_next(&prepasses, &iter, &key, (void**)&pi)) {
		pi->cb(pi->data, pfp);
	}
}

void RemovePrePass(char* name) {
	HT_delete(&prepasses, name);
}




// ---- timer queries ----


void checkQueries(DrawTimer* pd) {
	uint64_t p;
	uint64_t time;
	int tail;
	glexit("");
	
	//for(int j = 0; j < 5; j++) {
		//printf(" - [%d] = %d  %d\n", j, pd->timerEndIDs[j], pd->timerStartIDs[j]);
	//}
	
	while(pd->timerUsed > 0) {
		tail = (pd->timerNext - pd->timerUsed + 5) % 5; 
		//printf("tail: %d - %d = %d, %d\n",pd->timerNext, pd->timerUsed, tail, pd->timerEndIDs[tail]);
		// the end result being available should imply the start one is too
		// any ass-backwards hardware deserves the pipeline stall 
		glexit("");
		glGetQueryObjectui64v(pd->timerEndIDs[tail], GL_QUERY_RESULT_AVAILABLE, &p);
		if(GL_FALSE == p) {
			return; // the query isn't ready yet, so the ones after won't be either
		}
		
		//for(int j = 0; j < 5; j++) {
			//printf(" + [%d] = %d  %d\n", j, pd->timerEndIDs[j], pd->timerStartIDs[j]);
		//}
	
		glexit("");
		glGetQueryObjectui64v(pd->timerStartIDs[tail], GL_QUERY_RESULT, &time); 
		double ts = ((double)time) / 1000000000.0; // gpu time is in nanoseconds
		
		glGetQueryObjectui64v(pd->timerEndIDs[tail], GL_QUERY_RESULT, &time); 
		double te = ((double)time) / 1000000000.0;
		double t = te - ts;
		glexit("");
		
		pd->timerHistory[pd->timerHistIndex] = t;
		
		// calculate stats
		float a = 0;
		for(int i = 0; i < 16; i++) {
			pd->timerMin = fmin(pd->timerMin, pd->timerHistory[i]);
			pd->timerMax = fmax(pd->timerMax, pd->timerHistory[i]);
			a += pd->timerHistory[i];
		}
		pd->timerAvg = a / 16;
		 
		pd->timerHistIndex = (pd->timerHistIndex + 1) % 16;
		pd->timerUsed--;
		glexit("");
	}
}

void DrawTimer_Start(DrawTimer* pd) {
	if(pd->timerUsed < 5) {
		glQueryCounter(pd->timerStartIDs[pd->timerNext], GL_TIMESTAMP);
	}
	else {
		fprintf(stderr, "PassDrawable query queue exhausted \n");
	}	
}

void DrawTimer_End(DrawTimer* pd) {
	glQueryCounter(pd->timerEndIDs[pd->timerNext], GL_TIMESTAMP);
	
	pd->timerNext = (pd->timerNext + 1) % 5;
	pd->timerUsed++;
	
	checkQueries(pd);
}


void DrawTimer_Init(DrawTimer* pd) {
	
	memset(pd, 0, sizeof(*pd));
	
	glGenQueries(5, pd->timerStartIDs);
	glGenQueries(5, pd->timerEndIDs);
}

