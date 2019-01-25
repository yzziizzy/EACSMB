
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "game.h"
#include "gui.h"
#include "scene.h"
#include "shader.h"
#include "builder/render.h"

#include "c_json/json.h"
#include "json_gl.h"


GLuint fsQuadVAO, fsQuadVBO;
ShaderProgram* shadingProg;

extern GUIWindow* gwTest;
extern GUIBuilderControl* gbcTest;
extern RenderPipeline* rpipe;



void initFSQuad() {
	float vertices[] = {
		-1.0, -1.0, 0.0,
		-1.0, 1.0, 0.0,
		1.0, -1.0, 0.0,
		1.0, 1.0, 0.0
	};

	glGenVertexArrays(1, &fsQuadVAO);
	glBindVertexArray(fsQuadVAO);
	
	glGenBuffers(1, &fsQuadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fsQuadVBO);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void drawFSQuad() {
	
	glBindVertexArray(fsQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, fsQuadVBO);
	glexit("quad vbo");
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("quad draw");
	
}









void setupFBOs(GameState* gs, int resized) {
	int ww = gs->screen.wh.x;
	int wh = gs->screen.wh.y;
	
	if(gs->fboTextures) {
		destroyFBOTextures(gs->fboTextures);
		free(gs->fboTextures);
	}
	
	json_file_t* jsf = json_load_path("assets/config/fbo.json");
	
	json_value_t* tex;
	json_obj_get_key(jsf->root, "textures", &tex);
	
	FBOTexConfig texcfg2[6];
	unpack_fbo(tex, "diffuse", &texcfg2[0]);
	unpack_fbo(tex, "normal", &texcfg2[1]);
	unpack_fbo(tex, "selection", &texcfg2[2]);
	unpack_fbo(tex, "lighting", &texcfg2[3]);
	unpack_fbo(tex, "depth", &texcfg2[4]);
	texcfg2[5].internalType = 0;
	texcfg2[5].format = 0;
	texcfg2[5].size = 0;
	
	json_free(jsf->root);
	free(jsf->root);
	free(jsf);
	
	
	GLuint* texids = initFBOTextures(ww, wh, texcfg2);
	
	gs->diffuseTexBuffer = texids[0];
	gs->normalTexBuffer = texids[1];
	gs->selectionTexBuffer = texids[2];
	gs->lightingTexBuffer = texids[3];
	gs->depthTexBuffer = texids[4];
	
	//printf("New Main Depth: %d \n", texids[3]);
	
	// main gbuffer setup
	if(gs->gbuf.fb) { // evil abstraction breaking. meh.
		destroyFBO(&gs->gbuf);
	}
	
	FBOConfig gbufConf[] = {
		{GL_COLOR_ATTACHMENT0, gs->diffuseTexBuffer },
		{GL_COLOR_ATTACHMENT1, gs->normalTexBuffer },
		{GL_COLOR_ATTACHMENT2, gs->lightingTexBuffer },
		{GL_DEPTH_ATTACHMENT, gs->depthTexBuffer },
		{0,0}
	};
	
	initFBO(&gs->gbuf, gbufConf);
	
	// decal pass framebufer
	FBOConfig decalConf[] = {
		{GL_COLOR_ATTACHMENT0, gs->diffuseTexBuffer },
		{GL_COLOR_ATTACHMENT1, gs->normalTexBuffer },
		{GL_COLOR_ATTACHMENT2, gs->lightingTexBuffer },
		{GL_DEPTH_ATTACHMENT,  gs->depthTexBuffer },
		{0,0}
	};
	// depth buffer is also bound as a texture but disabled for writing
	
	initFBO(&gs->decalbuf, decalConf);
	
	// lighting pass framebufer
	FBOConfig lightingConf[] = {
		{GL_COLOR_ATTACHMENT0, gs->lightingTexBuffer },
		{GL_DEPTH_ATTACHMENT,  gs->depthTexBuffer },
		{0,0}
	};
	// depth buffer is also bound as a texture but disabled for writing
	
	initFBO(&gs->lightingbuf, lightingConf);
	
	// selection pass framebufer
	FBOConfig selectionConf[] = {
// 		{GL_COLOR_ATTACHMENT0, gs->diffuseTexBuffer },
// 		{GL_COLOR_ATTACHMENT1, gs->normalTexBuffer },
		{GL_COLOR_ATTACHMENT0, gs->selectionTexBuffer },
		{GL_DEPTH_ATTACHMENT,  gs->depthTexBuffer },
		{0,0}
	};
	// depth buffer is also bound as a texture but disabled for writing
	
	initFBO(&gs->selectionbuf, selectionConf);
	
	
	// pbo's for selection buffer
	gs->readPBO = -1;
	gs->activePBO = 0;
	
	if(gs->selectionPBOs[0]) {
		glDeleteBuffers(2, gs->selectionPBOs);
		glexit("");
	}
	
	glGenBuffers(2, gs->selectionPBOs);
	glexit("");
	
	glBindBuffer(GL_PIXEL_PACK_BUFFER, gs->selectionPBOs[0]);
	glexit("");
	glBufferData(GL_PIXEL_PACK_BUFFER, ww * wh * 4, NULL, GL_DYNAMIC_READ);
	glexit("");
	glBindBuffer(GL_PIXEL_PACK_BUFFER, gs->selectionPBOs[1]);
	glexit("");
	glBufferData(GL_PIXEL_PACK_BUFFER, ww * wh * 4, NULL, GL_DYNAMIC_READ);
	glexit("");
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	
	if(gs->selectionData) free(gs->selectionData);
	//printf("seldata size %d\n", ww * wh * 4);
	gs->selectionData = malloc(ww * wh * 4);
}




void initRenderLoop(GameState* gs) {
	
	// timer queries
	query_queue_init(&gs->queries.terrain);
	query_queue_init(&gs->queries.solids);
	query_queue_init(&gs->queries.selection);
	query_queue_init(&gs->queries.decals);
	query_queue_init(&gs->queries.emitters);
	query_queue_init(&gs->queries.effects);
	query_queue_init(&gs->queries.lighting);
	query_queue_init(&gs->queries.sunShadow);
	query_queue_init(&gs->queries.shading);
	query_queue_init(&gs->queries.gui);
	
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	setupFBOs(gs, 0);
		
	shadingProg = loadCombinedProgram("shading");
	
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sDiffuse"), 0);
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sNormals"), 1);
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sDepth"), 2);
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sSelection"), 3);
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sLighting"), 4);
	
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sShadow"), 5);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	initFSQuad();
	
	
	
}


void shadingPass(GameState* gs, PassFrameParams* pfp) {
	
	Matrix world, projView, viewWorld;
	
	world = IDENT_MATRIX;
	
	
	query_queue_start(&gs->queries.shading);
	
	glUseProgram(shadingProg->id);
	glexit("shading prog");

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, gs->diffuseTexBuffer);
	
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, gs->normalTexBuffer);

	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, gs->depthTexBuffer);

	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, gs->selectionTexBuffer);

	glActiveTexture(GL_TEXTURE0 + 4);
	glBindTexture(GL_TEXTURE_2D, gs->lightingTexBuffer);

	glActiveTexture(GL_TEXTURE0 + 5);
	//printf("btex: %d\n",gs->world->sunShadow->rpipe->backingTextures[0]);
	glBindTexture(GL_TEXTURE_2D, gs->world->sunShadow->rpipe->backingTextures[0]);

	
	glUniform1i(glGetUniformLocation(shadingProg->id, "debugMode"), gs->debugMode);
	glUniform2f(glGetUniformLocation(shadingProg->id, "clipPlanes"), gs->nearClipPlane, gs->farClipPlane);
// 	glUniform2f(glGetUniformLocation(shadingProg->id, "shadowClipPlanes"), gs->nearClipPlane, gs->farClipPlane);
	glUniform2f(glGetUniformLocation(shadingProg->id, "shadowClipPlanes"), gs->world->sunShadow->clipPlanes.x, gs->world->sunShadow->clipPlanes.y);
	
	glexit("shading samplers");
	
//	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "world"), 1, GL_FALSE, world.m);
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mViewProj"), 1, GL_FALSE, pfp->dp->mViewProj);
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mWorldView"), 1, GL_FALSE, pfp->dp->mWorldView);

	//mInverse(msGetTop(&gs->proj), &projView);
	//mInverse(msGetTop(&gs->view), &viewWorld);
	
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mProjView"), 1, GL_FALSE, pfp->dp->mProjView);
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mViewWorld"), 1, GL_FALSE, pfp->dp->mViewWorld);
	
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mWorldLight"), 1, GL_FALSE, &gs->world->sunShadow->mWorldLight);

	glexit("shading world");

	glUniform3fv(glGetUniformLocation(shadingProg->id, "sunNormal"), 1, (float*)&gs->sunNormal);
	
	if(gs->screen.resized) {
		glUniform2fv(glGetUniformLocation(shadingProg->id, "resolution"), 1, (float*)&gs->screen.wh);
	}
	
	drawFSQuad();
	glexit("post quad draw");
	
	query_queue_stop(&gs->queries.shading);
	
	
	query_queue_start(&gs->queries.gui);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	
	glDisable(GL_DEPTH_TEST);
	RenderPass_preFrameAll(gs->guiPass, pfp);
	RenderPass_renderAll(gs->guiPass, pfp->dp);
	RenderPass_postFrameAll(gs->guiPass);
	glEnable(GL_DEPTH_TEST);
	
	glDisable(GL_BLEND);
	
	query_queue_stop(&gs->queries.gui);
	
}


void selectionPass(XStuff* xs, GameState* gs, InputState* is, PassFrameParams* pfp) {
	
	glexit("");
	gs->lastSelectionFrame = gs->frameCount; 
	
	query_queue_start(&gs->queries.selection);
	
	glDepthFunc(GL_LESS);
	//glDisable(GL_DEPTH_TEST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, gs->selectionbuf.fb);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	//depthPrepass(xs, gs, is);
	
	
	RenderPass_preFrameAll(gs->world->terrainSelectionPass, pfp);
	RenderPass_renderAll(gs->world->terrainSelectionPass, pfp->dp);
	RenderPass_postFrameAll(gs->world->terrainSelectionPass);
	
	
	
	
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gs->selectionbuf.fb);
	
	
	
	// done drawing. initiate async pixel transfer
	
	
	
	//glEnable(GL_DEPTH_TEST);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	//printf("is buffer %d\n", gs->selectionPBOs[gs->activePBO]);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, gs->selectionPBOs[gs->activePBO]);
	
	glexit("selection buff");
	//printf("cursor pixels: %f, %f\n", is->cursorPosPixels.x, is->cursorPosPixels.y);
	
	glReadPixels(
		0, //is->cursorPosPixels.x,
		0, //is->cursorPosPixels.y,
		gs->screen.wh.x,
		gs->screen.wh.y,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		0);
	
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	glexit("read selection");
	glexit("");
	
	if(gs->selectionFence) glDeleteSync(gs->selectionFence);
	gs->selectionFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	
	gs->selectionFrame = gs->frameCount;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	query_queue_stop(&gs->queries.selection);
}



void renderDecals(XStuff* xs, GameState* gs, InputState* is, PassFrameParams* pfp) {
	/*
	glActiveTexture(GL_TEXTURE0 + 8);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, gs->depthTexBuffer);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sDepth"), 8);
	
	*/
	query_queue_start(&gs->queries.decals);
	World_drawDecals(gs->world, pfp);
	// drawTerrainRoads(gs->depthTexBuffer, &gs->scene.map, msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos, &gs->screen.wh);
	query_queue_stop(&gs->queries.decals);
	glexit("render decals");
}



void cleanUpView(XStuff* xs, GameState* gs, InputState* is) {
	msPop(&gs->view);
	msPop(&gs->proj);
	
	uniformBuffer_finish(&gs->perFrameUB);
	uniformBuffer_finish(&gs->perViewUB);
}



void SetUpPDP(GameState* gs, PassDrawParams* pdp) {
	
	pdp->mWorldView = msGetTop(&gs->view);
	pdp->mViewProj = msGetTop(&gs->proj);
	
	pdp->mProjView = &gs->invProj;
	pdp->mViewWorld = &gs->invView;
	
	mInverse(&pdp->mViewProj, &gs->invProj);
	mInverse(&pdp->mWorldView, &gs->invView);
	
	pdp->eyeVec = gs->eyeDir;
	pdp->eyePos = gs->eyePos;
	pdp->targetSize = (Vector2i){gs->screen.wh.x, gs->screen.wh.y};
	pdp->timeSeconds = (float)(long)gs->frameTime;
	pdp->timeFractional = gs->frameTime - pdp->timeSeconds;
	
}





#define PF_START(x) gs->perfTimes.x = getCurrentTime()
#define PF_STOP(x) gs->perfTimes.x = timeSince(gs->perfTimes.x)

void drawFrame(XStuff* xs, GameState* gs, InputState* is) {
	
	// draw the builder stuff first
	
// 	if(rpipe) {
// 		PassDrawParams rp;
// 		
// 		//rp.fboSize = (Vector2i){300,300};
// 		rp.mWorldView = msGetTop(&gs->view);
// 		rp.mViewProj = msGetTop(&gs->proj);
// 		
// 		// TODO actual inverse matrices
// 		rp.mViewWorld = msGetTop(&gs->view);
// 		rp.mProjView = msGetTop(&gs->proj);
// 		
// 		RenderPipeline_renderAll(rpipe, &rp);
//	}
// 	if(gbcTest) {
// 		PassDrawParams rp;
// 		
// 		//rp.fboSize = (Vector2i){300,300};
// 		rp.mWorldView = msGetTop(&gs->view);
// 		rp.mViewProj = msGetTop(&gs->proj);
// 		
// 		// TODO actual inverse matrices
// 		rp.mViewWorld = msGetTop(&gs->view);
// 		rp.mProjView = msGetTop(&gs->proj);
// 		
// 		RenderPipeline_renderAll(gbcTest->rpipe, &rp);
// 	}

	PassDrawParams pdp;
	//pdp.mWorldView = msGetTop(&gs->view);
	//pdp.mViewProj = msGetTop(&gs->proj);
	
	SetUpPDP(gs, &pdp);
	
	
	PassFrameParams pfp;
	pfp.dp = &pdp;
	pfp.timeElapsed = gs->frameSpan;
	pfp.gameTime = gs->frameTime; // this will get regenerated from save files later
	pfp.wallTime = gs->frameTime;
	
	

	
	
	RenderAllPrePasses(&pfp);
	
	
// 	printf("sm------\n");
	query_queue_start(&gs->queries.sunShadow);
//	ShadowMap_Render(gs->world->sunShadow, &pfp, &gs->sunNormal);
	query_queue_stop(&gs->queries.sunShadow);
// 	printf("sm^^^^^^\n");
	
// 	giShadowMap->customTexID = gs->world->sunShadow->depthTex; 
	
	glViewport(0, 0, gs->screen.wh.x, gs->screen.wh.y);
	
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW); // this is backwards, i think, because of the scaling inversion for z-up
	
	
	
	
	
	
	
	if(gs->hasMoved && gs->lastSelectionFrame < gs->frameCount - 8 && !gs->selectionPassDisabled) {
		//printf("doing selection pass %d\n", gs->frameCount);
		gs->hasMoved = 0;
		
		selectionPass(xs, gs, is, &pfp);
	}
	
	// update world state
// 	glBeginQuery(GL_TIME_ELAPSED, gs->queries.dtime[gs->queries.dtimenum]);
	//query_queue_start(&gs->queries.draw);

	
	glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	
	// terrain
	query_queue_start(&gs->queries.terrain);
	World_drawTerrain(gs->world, &pfp);
	query_queue_stop(&gs->queries.terrain);
	
	
	
	// decals
	glBindFramebuffer(GL_FRAMEBUFFER, gs->decalbuf.fb);
	glDepthMask(GL_FALSE); // disable depth writes for decals
	
	// hack
	gs->world->dm->dtex = gs->depthTexBuffer;
	gs->world->cdm->dtex = gs->depthTexBuffer;
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// only draw backfaces that are behind existing geometry
	glCullFace(GL_FRONT);
	glDepthFunc(GL_GREATER);
	
	renderDecals(xs, gs, is, &pfp);
	
	glDisable(GL_BLEND);
	glCullFace(GL_BACK);
	
	// back to normal gbuf for solids
	glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
	glDepthMask(GL_TRUE); // turn depth writes back on
	glDepthFunc(GL_LEQUAL); // normal depth function
	
	query_queue_start(&gs->queries.solids);
	World_drawSolids(gs->world, &pfp);
	query_queue_stop(&gs->queries.solids);
	
	
	// transparency and effects
	
 	query_queue_start(&gs->queries.effects);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE); // turn depth writes off
	
	World_preTransparents(gs->world, &pfp);
	
	// draw backfaces
	glCullFace(GL_FRONT);
	World_drawTransparents(gs->world, &pfp);
// 	
	// draw frontfaces
	glCullFace(GL_BACK);
	World_drawTransparents(gs->world, &pfp);
	
	World_postTransparents(gs->world);
	
	
	// emitters
// 	query_queue_start(&gs->queries.emitters);
	RenderPass_preFrameAll(gs->world->emitterPass, &pfp);
	RenderPass_renderAll(gs->world->emitterPass, pfp.dp);
	RenderPass_postFrameAll(gs->world->emitterPass);
// 	query_queue_stop(&gs->queries.emitters);
	
	glDepthMask(GL_TRUE); // turn depth writes back on
	glDisable(GL_BLEND);
	query_queue_stop(&gs->queries.effects);
	
	
	//renderFrame(xs, gs, is, &pfp);
	//query_queue_stop(&gs->queries.draw);
	

	
	
	

	
	
	// keep depth writes off for lighting
	
	// lighting
	// hacky code to adapt an isolated render pass outside a pipeline
	//glFrontFace(GL_CCW);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glDisable(GL_DEPTH_TEST);
	//glDepthFunc(GL_GREATER);
	query_queue_start(&gs->queries.lighting);
	glDepthMask(GL_FALSE); // no depth writes for light volumes
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
	
	glBindFramebuffer(GL_FRAMEBUFFER, gs->lightingbuf.fb);
	//glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
	
	gs->world->lm->dtex = gs->depthTexBuffer;
	RenderPass_preFrameAll(gs->world->lightingPass, &pfp);
	RenderPass_renderAll(gs->world->lightingPass, &pdp);
	RenderPass_postFrameAll(gs->world->lightingPass);
	
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	query_queue_stop(&gs->queries.lighting);
	
	//glDisable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glDepthMask(GL_TRUE);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	
	cleanUpView(xs, gs, is);
	
	
	// draw to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	shadingPass(gs, &pfp);
	
	
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	QuadTree_renderDebugVolumes(&gs->world->qt, &pfp);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	
	glXSwapBuffers(xs->display, xs->clientWin);
}



