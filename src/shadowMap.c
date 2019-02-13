


#include "shadowMap.h"
#include "debugWireframes.h"
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
	
	sm->clipPlanes.x = 1;
	sm->clipPlanes.y = 300;
}

void ShadowMap_addPass(ShadowMap* sm, RenderPass* pass) {
	VEC_PUSH(&sm->rpipe->passes, pass);
}



void ShadowMap_SetupFBOs(ShadowMap* sm) {	
	
	FBOTexConfig texcfg[] = {
	//	{ GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE }, // dummy
		{ GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT }, // depth
		{ 0, 0, 0}
	};
		
	RenderPipelineFBOConfig sbuf[] = {
		//{GL_COLOR_ATTACHMENT0, 0 }, // dummy
		{GL_DEPTH_ATTACHMENT, 0 },
		{0, -1}
	};
	
	
	RenderPipeline_setFBOTexConfig(sm->rpipe, texcfg);
	RenderPipeline_setFBOConfig(sm->rpipe, sbuf, "shadowmap");
	RenderPipeline_rebuildFBOs(sm->rpipe, sm->size);
	sm->depthTex = sm->rpipe->backingTextures[0];
	
	
	
	/*
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
	*/
}




// lightPos for now is the worldspace direction towards the directional light 

void ShadowMap_Render(ShadowMap* sm, PassFrameParams* cameraPFP, Vector* lightPos) {
	
	
	//return;
	
	// figure out a good view
	PassDrawParams* cameraPDP = cameraPFP->dp;
	PassFrameParams smPFP;
	PassDrawParams smPDP;
	smPFP.dp = &smPDP;
	
	Vector lpos;
	Vector ldir;
	Vector lcenter = {120, 120, 0};
	
	// place the directional light some arbitrary distance away
	//vNorm(lightPos, &ldir);
	//vScale(&ldir, 200, &lpos); 
	
	
	
	//lpos = (Vector){0,50, 0};
	
	// BUG: this whole calculation is very broken
	Matrix m_wv = IDENT_MATRIX;
	Matrix m_vp = IDENT_MATRIX;
	Matrix m_wp = IDENT_MATRIX;
	
	Matrix m_wv_inv;
	Matrix m_vp_inv;
	Matrix m_wp_inv;
	
	mOrtho(-100, 100, 100, -100, sm->clipPlanes.x, sm->clipPlanes.y, &m_vp);
	//mLookAt(&lpos, &(Vector){10,0,10}, &(Vector){0,1,0}, &m_wv);
	
	//mPerspective(60, 1, sm->clipPlanes.x, sm->clipPlanes.y, &m_vp);
	
	
	double time = cameraPDP->timeSeconds + cameraPDP->timeFractional;
	
	float zoom = (fabs(sin(fmod(time, 6.28))) * -100) - 150;
	float lrot = -3.14 / 2; //fmod(time / 3, 6.28);
	zoom = -150;
	
	mTrans3f(0, -1, zoom, &m_wv);
	mRot3f(1, 0, 0, F_PI / 6, &m_wv);
	mRot3f(0,1,0, lrot, &m_wv);
	mTrans3f(-lcenter.x, 0, -lcenter.y, &m_wv);
	
		// y-up to z-up rotation
	mRot3f(1, 0, 0, F_PI_2, &m_wv);
	mScale3f(1, 1, -1, &m_wv);
	
	mFastMul(&m_wv, &m_vp, &m_wp);
	
	sm->mWorldLight = m_wp;
	
	// inverses shouldn't be used by any shadow shaders
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
	
	
	debugWF_ProjMatrix(&m_wp);
	
	glViewport(0, 0, sm->size.x, sm->size.y);
// 	glViewport(0, 0, 1024, 1024);
	
	
	RenderPipeline_renderAll(sm->rpipe, &smPFP);
// 	RenderPipeline_renderAll(sm->rpipe, cameraPFP);
}











