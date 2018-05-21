
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






static void unpack_fbo(json_value_t* p, char* key, FBOTexConfig* cfg) {
	char* a, *b, *c;
	json_value_t* o, *v1, *v2, *v3;
	
	json_obj_get_key(p, key, &o);
	
	json_obj_get_key(o, "internalType", &v1); a = v1->v.str;
	json_as_GLenum(v1, &cfg->internalType);
	
	json_obj_get_key(o, "format", &v2); b = v2->v.str;
	json_as_GLenum(v2, &cfg->format);
	
	json_obj_get_key(o, "size", &v3); c = v3->v.str;
	json_as_GLenum(v3, &cfg->size);
	
	//printf("fbo cfg from json: %s: %x, %s: %x, %s: %x\n", a, cfg->internalType, b, cfg->format, c, cfg->size);
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
	query_queue_init(&gs->queries.draw);
	query_queue_init(&gs->queries.selection);
	query_queue_init(&gs->queries.emitters);
	
	
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
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	
	initFSQuad();
	
	
	
}


void shadingPass(GameState* gs) {
	
	Matrix world, projView, viewWorld;
	
	world = IDENT_MATRIX;
	
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

	
	glUniform1i(glGetUniformLocation(shadingProg->id, "debugMode"), gs->debugMode);
	glUniform2f(glGetUniformLocation(shadingProg->id, "clipPlanes"), gs->nearClipPlane, gs->farClipPlane);
	
	glexit("shading samplers");
	
//	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "world"), 1, GL_FALSE, world.m);
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mViewProj"), 1, GL_FALSE, msGetTop(&gs->proj)->m);
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mWorldView"), 1, GL_FALSE, msGetTop(&gs->view)->m);

	mInverse(msGetTop(&gs->proj), &projView);
	mInverse(msGetTop(&gs->view), &viewWorld);
	
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mProjView"), 1, GL_FALSE, projView.m);
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "mViewWorld"), 1, GL_FALSE, viewWorld.m);

	glexit("shading world");

	glUniform3fv(glGetUniformLocation(shadingProg->id, "sunNormal"), 1, (float*)&gs->sunNormal);
	
	if(gs->screen.resized) {
		glUniform2fv(glGetUniformLocation(shadingProg->id, "resolution"), 1, (float*)&gs->screen.wh);
	}
	
	drawFSQuad();
	glexit("post quad draw");
	
	
	gui_RenderAll(gs);
	
}


void selectionPass(XStuff* xs, GameState* gs, InputState* is) {

	gs->lastSelectionFrame = gs->frameCount; 
	
	query_queue_start(&gs->queries.selection);
	
	glDepthFunc(GL_LESS);
	//glDisable(GL_DEPTH_TEST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, gs->selectionbuf.fb);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	depthPrepass(xs, gs, is);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gs->selectionbuf.fb);
	
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



void renderFrame(XStuff* xs, GameState* gs, InputState* is) {
	

	
	// draw terrain
// 	drawTerrainBlock(&gs->map, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos);
	//drawTerrain(&gs->scene.map, &gs->perViewUB, &gs->cursorPos, &gs->screen.wh);
	
	World_drawTerrain(gs->world);
	
	//renderMarker(gs, 0,0);

	World_drawSolids(gs->world, msGetTop(&gs->view), msGetTop(&gs->proj));

	
	//drawStaticMesh(gs->world->testmesh.sm, msGetTop(&gs->view), msGetTop(&gs->proj));
	
/*
	gui_RenderAll(gs);
	*/

}


void renderDecals(XStuff* xs, GameState* gs, InputState* is) {
	/*
	glActiveTexture(GL_TEXTURE0 + 8);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, gs->depthTexBuffer);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sDepth"), 8);
	
	*/
	
	World_drawDecals(gs->world,  msGetTop(&gs->view), msGetTop(&gs->proj));
	// drawTerrainRoads(gs->depthTexBuffer, &gs->scene.map, msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos, &gs->screen.wh);
	
	glexit("render decals");
}


void renderParticles(XStuff* xs, GameState* gs, InputState* is) {
	//Draw_Emitter(dust, msGetTop(&gs->view), msGetTop(&gs->proj), gs->frameTime);
	
	glexit("render particles");
}




void cleanUpView(XStuff* xs, GameState* gs, InputState* is) {
	msPop(&gs->view);
	msPop(&gs->proj);
	
	uniformBuffer_finish(&gs->perFrameUB);
	uniformBuffer_finish(&gs->perViewUB);
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
	
	PassFrameParams pfp;
	pfp.timeElapsed = gs->frameSpan;
	pfp.gameTime = gs->frameTime;
	pfp.wallTime = 0;
	
	RenderAllPrePasses(&pfp);
	
	
	
	if(gs->hasMoved && gs->lastSelectionFrame < gs->frameCount - 8 && !gs->selectionPassDisabled) {
		//printf("doing selection pass %d\n", gs->frameCount);
		gs->hasMoved = 0;
		
		selectionPass(xs, gs, is);
	}
	
	// update world state
// 	glBeginQuery(GL_TIME_ELAPSED, gs->queries.dtime[gs->queries.dtimenum]);
	query_queue_start(&gs->queries.draw);

	
	glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// clear color buffer for actual rendering
	//glClear(GL_COLOR_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	
	renderFrame(xs, gs, is);
	query_queue_stop(&gs->queries.draw);
	
	// decals
	glBindFramebuffer(GL_FRAMEBUFFER, gs->decalbuf.fb);
	
	glDepthMask(GL_FALSE); // disable depth writes for decals
	
	renderDecals(xs, gs, is);
	
	
	query_queue_start(&gs->queries.emitters);
	glexit("");
	renderParticles(xs, gs, is); // particles are alpha blended and need to be on top of decals
	query_queue_stop(&gs->queries.emitters);
	
	// keep depth writes off for lighting
	
	// lighting
	// hacky code to adapt an isolated render pass outside a pipeline
	//glFrontFace(GL_CCW);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glDisable(GL_DEPTH_TEST);
	//glDepthFunc(GL_GREATER);
	
	glBindFramebuffer(GL_FRAMEBUFFER, gs->lightingbuf.fb);
	//glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
	PassDrawParams pdp;
	pdp.mWorldView = msGetTop(&gs->view); 
	pdp.mViewProj = msGetTop(&gs->proj);
	
	gs->world->lm->dtex = gs->depthTexBuffer;
	RenderPass_preFrameAll(gs->world->lightingPass, &pfp);
	RenderPass_renderAll(gs->world->lightingPass, &pdp);
	RenderPass_postFrameAll(gs->world->lightingPass);
	
	//glDisable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glDepthMask(GL_TRUE);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	
	cleanUpView(xs, gs, is);
	
	
	// draw to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	shadingPass(gs);
	
	glXSwapBuffers(xs->display, xs->clientWin);
}



