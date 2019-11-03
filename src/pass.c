
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




static void shading_pass_render(RenderPipeline* rpipe, GLuint progID, PassDrawParams* pdp) {
// 	ShaderProgram* prog = pd->prog;
	
//	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "world"), 1, GL_FALSE, world.m);
	glUniformMatrix4fv(glGetUniformLocation(progID, "mViewProj"), 1, GL_FALSE, pdp->mWorldView->m);
	glUniformMatrix4fv(glGetUniformLocation(progID, "mWorldView"), 1, GL_FALSE, pdp->mViewProj->m);
	glexit("");
	
// 	mInverse(msGetTop(&gs->proj), &projView);
// 	mInverse(msGetTop(&gs->view), &viewWorld);
	
	glUniformMatrix4fv(glGetUniformLocation(progID, "mProjView"), 1, GL_FALSE, pdp->mProjView->m);
	glUniformMatrix4fv(glGetUniformLocation(progID, "mViewWorld"), 1, GL_FALSE, pdp->mViewWorld->m);
	glexit("");
	
// 	glUniform3fv(glGetUniformLocation(shadingProg->id, "sunNormal"), 1, (float*)&gs->sunNormal);
	
// 	glUniform2iv(glGetUniformLocation(prog->id, "resolution"), 1, (int*)&rp->fboSize);
	glUniform2f(glGetUniformLocation(progID, "resolution"), rpipe->viewSz.x, rpipe->viewSz.x);
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
	
	
	RenderPass_init(pass);
	
	PassDrawable* d = Pass_allocDrawable("shading");
	d->draw = (void*)shading_pass_render;
	d->prog = loadCombinedProgram(shaderName);
	d->data = rpipe;
	
	VEC_PUSH(&pass->drawables, d);
	
	
	VEC_PUSH(&rpipe->passes, pass);
	
	
}




// save copies of the fbo configuration so locals can be fed in with ease.
void RenderPipeline_setFBOConfig(RenderPipeline* rp, RenderPipelineFBOConfig* cfg, char* name) {
	int i;
	for(i = 0; cfg[i].texIndex > -1; i++);
	
	printf("rp config len: %d\n", i);
	
	RenderPipelineFBOConfig* c = calloc(1, (i + 1) * sizeof(*c));
	memcpy(c, cfg, (i + 1) * sizeof(*c));
	
	VEC_PUSH(&rp->fboConfig, c);
}

// save copies of the fbo configuration so locals can be fed in with ease.
void RenderPipeline_setFBOTexConfig(RenderPipeline* rp, FBOTexConfig* texcfg) {
	
	int i;
	
	// first copy the backing texture config
	for(i = 0; texcfg[i].size != 0; i++);
	rp->fboTexConfig = calloc(1, sizeof(*rp->fboTexConfig) * (i + 1));
	memcpy(rp->fboTexConfig, texcfg, sizeof(*rp->fboTexConfig) * (i + 1));
}



void RenderPipeline_rebuildFBOs(RenderPipeline* rp, Vector2i sz) {
	int i;
	
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
		
		// TODO: BUG
// 		VEC_LOOP(&rp->fboConfig, ind) destroyFBO(VEC_ITEM(&rp->fboConfig, ind));
	}
	
	
	rp->backingTextures = initFBOTextures(rp->viewSz.x, rp->viewSz.y, rp->fboTexConfig);
	rp->fbos = calloc(1, sizeof(*rp->fbos) * VEC_LEN(&rp->fboConfig));
	
	
	VEC_EACH(&rp->fboConfig, ind, cfg) {
		int len;
		for(len = 0; cfg[len].texIndex > -1; len++);
		
		FBOConfig* realCfg = calloc(1, (len + 1) * sizeof(*realCfg));
		for(i = 0; i < len; i++) {
			realCfg[i].attachment = cfg[i].attachment;
			realCfg[i].texture = rp->backingTextures[cfg[i].texIndex];
		}
	
		rp->fbos[ind] = allocFBO();
		initFBO(rp->fbos[ind], realCfg);
		
		free(realCfg);
	}
	
}



void RenderPass_init(RenderPass* rp) {
	
	// disable fiddling for now
	rp->drawBuffer = GL_INVALID_INDEX;
	rp->readBuffer = GL_INVALID_INDEX;
	
	// these will generate harmless errors if the uniform is not found
	//bp->diffuseUL = glGetUniformLocation(prog->id, "sDiffuse");
	//bp->normalsUL = glGetUniformLocation(prog->id, "sNormals");
	//bp->lightingUL = glGetUniformLocation(prog->id, "sLighting");
	//bp->depthUL = glGetUniformLocation(prog->id, "sDepth");
	//glerr("harmless");
}


void RenderPipeline_destroy(RenderPipeline* rp) {
	
	if(rp->backingTextures) {
		destroyFBOTextures(rp->backingTextures);
		free(rp->backingTextures);
		
		// TODO: fix
		destroyFBO(rp->fbos[0]);
		destroyFBO(rp->fbos[1]);
	}
	
	// TODO: clean up all the children of the pass
}





void RenderPipeline_renderAll(RenderPipeline* bp, PassFrameParams* pfp) {
	
	if(!bp->backingTextures) return;
	
	
	// TODO: figure out where this should be
	VEC_EACH(&bp->passes, ind, pass) {
		RenderPass_preFrameAll(pass, pfp);
	}
	
	
	for(int i = 0; i < VEC_LEN(&bp->passes); i++) {
		RenderPass* pass = VEC_ITEM(&bp->passes, i);
		//ShaderProgram* prog = pass->prog;
		
		glBindFramebuffer(GL_FRAMEBUFFER, bp->fbos[pass->fboIndex]->fb);
		
		if(pass->readBuffer != GL_INVALID_INDEX) glReadBuffer(pass->readBuffer);
		if(pass->drawBuffer != GL_INVALID_INDEX) glDrawBuffer(pass->drawBuffer);
		
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		//printf("fbo bound: %d\n", bp->fbos[pass->fboIndex]->fb);
		
		int clear = 0;
		if(pass->clearColor) clear |= GL_COLOR_BUFFER_BIT;
		if(pass->clearDepth) clear |= GL_DEPTH_BUFFER_BIT;
		if(clear) {
			//printf("clearing builder fbo\n");
			glClearColor(bp->clearColor.x, bp->clearColor.y, bp->clearColor.z, bp->clearColor.w);
			glClear(clear);
		}
		
		// TODO: ensure the backing textures are not bound to an active fbo
		
//		 glEnable (GL_BLEND);
		// glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glDepthFunc(GL_LEQUAL);
// 		glDepthMask(GL_TRUE);
		
		RenderPass_renderAll(bp, pass, pfp->dp);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	
	
	// TODO: figure out where this should be
	VEC_EACH(&bp->passes, ind, pass) {
		RenderPass_postFrameAll(pass);
	}
}


int RenderPass_addDrawable(RenderPass* rp, PassDrawable* d) {
	
	if(!d->prog) {
		fprintf(stderr, "!!! RenderPass_addDrawable: missing shader.\n");
		return 1;
	}

#define GET_UNIFORM(ul) \
	d->ul_##ul = glGetUniformLocation(d->prog->id, #ul)
	
	GET_UNIFORM(mWorldView);
	GET_UNIFORM(mViewProj);
	GET_UNIFORM(mWorldProj);
	
	GET_UNIFORM(mViewWorld);
	GET_UNIFORM(mProjView);
	GET_UNIFORM(mProjWorld);
	
	GET_UNIFORM(timeSeconds);
	GET_UNIFORM(timeFractional);
	
	GET_UNIFORM(targetSize);
	
	
// 	d->diffuseUL = glGetUniformLocation(d->prog->id, "sDiffuse");
// 	d->normalsUL = glGetUniformLocation(d->prog->id, "sNormals");
// 	d->lightingUL = glGetUniformLocation(d->prog->id, "sLighting");
// 	d->depthUL = glGetUniformLocation(d->prog->id, "sDepth");
	glerr("harmless");
	
	VEC_PUSH(&rp->drawables, d);
}

static void bindUniforms(PassDrawable* d, PassDrawParams* pdp) {
#define BIND_MATRIX(ul, val) \
	if(d->ul_##ul != -1) glUniformMatrix4fv(d->ul_##ul, 1, GL_FALSE, val)
	
	BIND_MATRIX(mWorldView, pdp->mWorldView->m);
	BIND_MATRIX(mViewProj, pdp->mViewProj->m);
	BIND_MATRIX(mWorldProj, pdp->mWorldProj->m);
	
	BIND_MATRIX(mViewWorld, pdp->mViewWorld->m);
	BIND_MATRIX(mProjView, pdp->mProjView->m);
	BIND_MATRIX(mProjWorld, pdp->mProjWorld->m);
	
	if(d->ul_timeSeconds != -1) glUniform1f(d->ul_timeSeconds, pdp->timeSeconds);
	if(d->ul_timeFractional != -1) glUniform1f(d->ul_timeFractional, pdp->timeFractional);
	
	if(d->ul_targetSize != -1) glUniform2iv(d->ul_targetSize, 1, (GLint*)&pdp->targetSize);
	
#undef BIND_MATRIX
}

static void bindFBOUniforms(RenderPipeline* rpipe, PassDrawable* d) {
	
	
	for(int i = 0; rpipe->fboTexConfig[i].internalType != 0; i++) {
		FBOTexConfig* cfg = &rpipe->fboTexConfig[i];
		if(!cfg->uniformName) continue;
		
// 		glBindTexture(GL_TEXTURE_2D, bp->backingTextures[DIFFUSE]);

		GLuint loc = glGetUniformLocation(d->prog->id, cfg->uniformName);
		if(!loc) continue;
		
		GLuint64 texHandle = glGetTextureHandleARB(rpipe->backingTextures[i]);
		glexit("");
		
		if(!glIsTextureHandleResidentARB(texHandle)) {
			glMakeTextureHandleResidentARB(texHandle);
		}
		glexit("");
		
		glUniformHandleui64ARB(loc, texHandle);
		glexit("");
	}
}


void RenderPass_renderAll(RenderPipeline* rpipe, RenderPass* pass, PassDrawParams* pdp) {
	
	int i;
	
	for(i = 0; i < VEC_LEN(&pass->drawables); i++) {
		PassDrawable* d = VEC_ITEM(&pass->drawables, i);
		
		DrawTimer_Start(&d->timer);
		
		glUseProgram(d->prog->id);
		if(rpipe) bindFBOUniforms(rpipe, d);
		bindUniforms(d, pdp);
		
		// BUG the code below works but the declarations do not agree. dunno which one is wrong.
		d->draw(d->data, d->prog->id, pdp);
		
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
	VEC_INIT(&rp->fboConfig);
	
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
	
	d->ul_mWorldView = -1;
	d->ul_mViewProj = -1;
	d->ul_mWorldProj = -1;
	d->ul_mViewWorld = -1;
	d->ul_mProjView = -1;
	d->ul_mProjWorld = -1;
	d->ul_timeSeconds = -1;
	d->ul_timeFractional = -1;
	d->ul_targetSize = -1;
	
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



// ---- pdp utilities ----


void PassDrawParams_DeepCopy(PassDrawParams* orig, PassDrawParams* copy) {
	
	*copy = *orig;
	
#define COPY_MATRIX(f) copy->f = malloc(sizeof(*copy->f)); *copy->f = *orig->f;
	
	COPY_MATRIX(mWorldView);
	COPY_MATRIX(mViewProj);
	COPY_MATRIX(mWorldProj);
	
	COPY_MATRIX(mViewWorld);
	COPY_MATRIX(mProjView);
	COPY_MATRIX(mProjWorld);
	
#undef COPY_MATRIX
}


void PassFrameParams_DeepCopy(PassFrameParams* orig, PassFrameParams* copy) {
	
	*copy = *orig;
	
	copy->dp = malloc(sizeof(*copy->dp));
	PassDrawParams_DeepCopy(orig->dp, copy->dp);
}



void PassDrawParams_DeepFree(PassDrawParams* pdp) {
#define FREE_MATRIX(f) if(pdp->f) free(pdp->f);
	FREE_MATRIX(mWorldView);
	FREE_MATRIX(mViewProj);
	FREE_MATRIX(mWorldProj);
	FREE_MATRIX(mViewWorld);
	FREE_MATRIX(mProjView);
	FREE_MATRIX(mProjWorld);
#undef FREE_MARIX
}

void PassFrameParams_DeepFree(PassFrameParams* pfp) {
	if(pfp->dp) {
		PassDrawParams_DeepFree(pfp->dp);
		free(pfp->dp);
	}
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

