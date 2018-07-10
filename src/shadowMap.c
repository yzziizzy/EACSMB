


#include "shadowMap.h"
#include "game.h"
#include "fbo.h"
#include "c_json/json.h"






void ShadowMap_init(ShadowMap* sm) {
	
	
}


void ShadowMap_SetupFBOs(ShadowMap* sm, GameState* gs) {
	
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

	texcfg2[5].internalType = 0;
	texcfg2[5].format = 0;
	texcfg2[5].size = 0;
	
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






void ShadowMap_Render(ShadowMap* sm, PassDrawParams* cameraPDP, Vector* lightPos) {
	
	// figure out a good view
	PassDrawParams smPDP;
	
	glViewport(0, 0, sm->size.x, sm->size.y);
	
	glBindFramebuffer(GL_FRAMEBUFFER, sm->fb.fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, sm->depthTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	
	glClear(GL_DEPTH_BUFFER_BIT);
	
	
	RenderPipeline_renderAll(sm->rpipe, &smPDP);
	
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	
}











