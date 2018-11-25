


#include "shadowMap.h"
#include "game.h"
#include "fbo.h"
#include "c_json/json.h"






ShadowMap* ShadowMap_alloc() {
	ShadowMap* sm;
	
	pcalloc(sm);
	ShadowMap_init(sm);
	
	return sm;
}

void ShadowMap_init(ShadowMap* sm) {
	pcalloc(sm->rpipe);
	
	RenderPipeline_init(sm->rpipe);
	
}

void ShadowMap_addPass(ShadowMap* sm, RenderPass* pass) {
	VEC_PUSH(&sm->rpipe->passes, pass);
}



void ShadowMap_SetupFBOs(ShadowMap* sm) {	
	//if(gs->fboTextures) {
		//destroyFBOTextures(gs->fboTextures);
		//free(gs->fboTextures);
	//}
	if(sm->depthTex) {
		glDeleteTextures(1, &sm->depthTex);
	}
	
	json_file_t* jsf = json_load_path("assets/config/shadows.json");
	
	json_value_t* tex;
	json_obj_get_key(jsf->root, "textures", &tex);
	
	FBOTexConfig texcfg2[2];
	unpack_fbo(tex, "depth", &texcfg2[0]);

	texcfg2[1].internalType = 0;
	texcfg2[1].format = 0;
	texcfg2[1].size = 0;
	
	json_free(jsf->root);
	free(jsf->root);
	free(jsf);
	

	GLuint* texids = initFBOTextures(sm->size.x, sm->size.y, texcfg2);
	
	sm->depthTex = texids[0];


	
	//printf("New Main Depth: %d \n", texids[3]);
	
	// main gbuffer setup
	if(sm->fb.fb) { // evil abstraction breaking. meh.
		destroyFBO(&sm->fb);
	}
	
	FBOConfig gbufConf[] = {
		{GL_DEPTH_ATTACHMENT, sm->depthTex },
		{0,0}
	};
	
	initFBO(&sm->fb, gbufConf);
	
	
	
}




// lightPos for now is the worldspace direction towards the directional light 

void ShadowMap_Render(ShadowMap* sm, PassDrawParams* cameraPDP, Vector* lightPos) {
	
	
	//return;
	
	// figure out a good view
	PassDrawParams smPDP;
	
	Vector lpos;
	Vector ldir;
	
	// place the directional light some arbitrary distance away
	vNorm(lightPos, &ldir);
	vScale(&ldir, 200, &lpos); 
	
	
	Matrix m_wv;
	Matrix m_vp;
	Matrix m_wp;
	
	Matrix m_wv_inv;
	Matrix m_vp_inv;
	Matrix m_wp_inv;
	
	mOrtho(-1, 1, 1, -1, 5, 500, &m_vp);
	mLookAt(&lpos, &(Vector){0,0,0}, &(Vector){0,0,1}, &m_wv);
	
	mFastMul(&m_vp, &m_wv, &m_wp);
	
	mInverse(&m_wv, &m_wv_inv);
	mInverse(&m_vp, &m_vp_inv);
	mInverse(&m_wp, &m_wp_inv);
	
	
	smPDP.mWorldView = &m_wv;
	smPDP.mViewProj = &m_vp;
	smPDP.mWorldProj = &m_wp;
	
	smPDP.mViewWorld = &m_wv_inv;
	smPDP.mProjView = &m_vp_inv;
	smPDP.mProjWorld = &m_wp_inv;
	
	smPDP.eyeVec = ldir;
	smPDP.eyePos = lpos;
	
	smPDP.targetSize = sm->size;
	
	smPDP.timeSeconds = cameraPDP->timeSeconds;
	smPDP.timeFractional = cameraPDP->timeFractional;
	
	
	
	glViewport(0, 0, sm->size.x, sm->size.y);
	
	glBindFramebuffer(GL_FRAMEBUFFER, sm->fb.fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sm->depthTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	glClear(GL_DEPTH_BUFFER_BIT);
	
	
	RenderPipeline_renderAll(sm->rpipe, &smPDP);
	
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	
}











