
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>


#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"
#include "text/text.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"
#include "window.h"
#include "map.h"
#include "scene.h"
#include "game.h"



GLuint vao, vbo, ibo;
GLuint proj_ul, view_ul, model_ul; 

Matrix mProj, mView, mModel;

float angle, zoom;



// temp shit
TextRes* arial;
ShaderProgram* textProg;
Matrix textProj, textModel;
TextRenderInfo* strRI;
Texture* cnoise;

MapBlock* map;
TerrainBlock* terrain;

void initPatch();
void drawPatch();

void initGame(XStuff* xs, GameState* gs) {
// 	printMapMemoryStats();
	
	glerr("left over error on game init");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	glerr("clearing before program load");
	gs->tileProg = loadProgram("tiles", "tiles", NULL, "tiles", "tiles");
	
	
	model_ul = glGetUniformLocation(gs->tileProg->id, "mModel");
	glerr("uniform loc Model");
	view_ul = glGetUniformLocation(gs->tileProg->id, "mView");
	glerr("uniform loc View");
	proj_ul = glGetUniformLocation(gs->tileProg->id, "mProj");
	glerr("uniform loc Projection");
	
	
	// set up matrix stacks
	MatrixStack* model, *view, *proj;
	
	model = &gs->model;
	view = &gs->view;
	proj = &gs->proj;
	
	msAlloc(30, model);
	msAlloc(2, view);
	msAlloc(2, proj);

	msIdent(model);
	msIdent(view);
	msIdent(proj);
	
	msScale3f(1024,1024,1024, model);


	
	
	// perspective matrix, pretty standard
	msPerspective(60, 1.0, 0.0001f, 10.0f, proj);
	
	gs->zoom = -960.0;
	gs->direction = 0.0;
	angle = 0.2;
	
	// view matrix, everything is done backwards
//  	msScale3f(.8, .8, .8, view); // legacy
	
	
	
//	initPatch();
	initTerrain();
	
	
// 	map = allocMapBlock(sizeof(float), 1024, 1024);
	
	terrain = allocTerrainBlock(0, 0);
	updateTerrainTexture(terrain);
	
// 	mProj = IDENT_MATRIX;
// 	mView = IDENT_MATRIX;
// 	mModel = IDENT_MATRIX;
	
	GLint maxtes;
	glGetIntegerv(GL_MAX_TESS_GEN_LEVEL, &maxtes);
	printf("max tessellation level: %d\n", maxtes);

	// text rendering stuff
	arial = LoadFont("/usr/share/fonts/corefonts/arial.ttf", 64, NULL);
	glerr("clearing before text program load");
	textProg = loadProgram("text", "text", NULL, NULL, NULL);
	
	unsigned int colors[] = {
		0xFF0000FF, 2,
		0x00FF00FF, 4,
		0x0000FFFF, INT_MAX
	};
	
	strRI = prepareText(arial, "FPS: --", -1, colors);
	
	
	
}


double getCurrentTime() {
	double now;
	struct timespec ts;
	static double offset = 0;
	
	
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	
	now = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
	if(offset == 0) offset = now;
	
	return now - offset;
}


float rot = 0;

void renderFrame(XStuff* xs, GameState* gs, InputState* is) {
	
	//mModel = IDENT_MATRIX;
	
	char frameCounterBuf[128];
	unsigned int fpsColors[] = {0xeeeeeeff, 4, 0xeeee22ff, INT_MAX};
	
	
	static int frameCounter = 0;
	static float last_frame = 0;
	
	float now = getCurrentTime();

	if (last_frame == 0)
		last_frame = now;
	
	gs->frameTime = (float)now;
	float te = gs->frameSpan = (float)(now - last_frame);
	
	frameCounter = (frameCounter + 1) % 60;
	
	static float lastPoint = 0;
	if(lastPoint == 0.0f) lastPoint = gs->frameTime;
	if(frameCounter == 0) {
		float fps = 60.0f / (gs->frameTime - lastPoint);
		
		snprintf(frameCounterBuf, 128, "FPS:  %.2f", fps);
		
		//printf("--->%s\n", frameCounterBuf);
		
		updateText(strRI, frameCounterBuf, -1, fpsColors);
		
		lastPoint = now;
	}
	
	
	// look direction
	if(is->keyState[38] & IS_KEYDOWN) {
		rot +=  .08 * te; // 45.0f * ((float)(Now - LastTime) / 1);
	}
	if(is->keyState[39] & IS_KEYDOWN) {
		rot -=  .08 * te; // 45.0f * ((float)(Now - LastTime) / 1);
	}
	
	// zoom
	if(is->keyState[52] & IS_KEYDOWN) {
		gs->zoom +=  .5 * te; // 45.0f * ((float)(Now - LastTime) / 1);
 		gs->zoom = fmin(gs->zoom, -10.0);
		printf("zoom: %f\n", gs->zoom);
	}
	if(is->keyState[53] & IS_KEYDOWN) {
		gs->zoom -=  .5 * te; // 45.0f * ((float)(Now - LastTime) / 1);
	}

	// movement
	if(is->keyState[113] & IS_KEYDOWN) {
		gs->lookCenter.x -=  .5 * te; // 45.0f * ((float)(Now - LastTime) / 1);
	}
	if(is->keyState[114] & IS_KEYDOWN) {
		gs->lookCenter.x +=  .5 * te; // 45.0f * ((float)(Now - LastTime) / 1);
	}
	
	if(is->keyState[111] & IS_KEYDOWN) {
		gs->lookCenter.y -=  .5 * te; // 45.0f * ((float)(Now - LastTime) / 1);
	}
	if(is->keyState[116] & IS_KEYDOWN) {
		gs->lookCenter.y +=  .5 * te; // 45.0f * ((float)(Now - LastTime) / 1);
	}
	
	angle = (rot * 3.14159265358979) / 180 ;
	//mScale3f(10, 10, 10, &mModel);
	//mRot3f(0, 1, 0, angle, &mModel);
	msPush(&gs->view);
	
	
	
	// order matters! don't mess with this.
	msTrans3f(0, -1, gs->zoom, &gs->view);
	msRot3f(1, 0, 0, 3.1415/6, &gs->view);
	msRot3f(0,1,0, angle, &gs->view);
	msTrans3f(gs->lookCenter.x, 0, gs->lookCenter.y, &gs->view);
	// TODO: fix coordinates

	
 	msPush(&gs->model);
	
	// move it to the middle of the screen
	msTrans3f(-.5, 0, -.5, &gs->model);
	
	// y-up to z-up rotation
	msRot3f(1, 0, 0, 3.1415/2, &gs->model);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glerr("clearing");
	
// 	GLint MaxPatchVertices = 0;
// 	glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
// 	
// 	printf("Max supported patch vertices %d\n", MaxPatchVertices);
// 	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	
	//glUseProgram(gs->tileProg->id);
	// set up matrices
/*	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, mProj.m);
	glUniformMatrix4fv(model_ul, 1, GL_FALSE, mModel.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, mView.m);
	glexit("uniform locations");
/*/// 	printf("%d %d %d \n", proj_ul, model_ul, view_ul);
	
	//drawPatch();

	// draw "tiles"
	
	drawTerrainBlock(terrain, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj));
	
	
	msPop(&gs->model);
	msPop(&gs->view);
	
	glUseProgram(textProg->id);
	
	
	
	// text stuff
	textProj = IDENT_MATRIX;
	textModel = IDENT_MATRIX;
	
	mOrtho(0, 1, 0, 1, -1, 100, &textProj);
	//mScale3f(.5,.5,.5, &textProj);
	
	mScale3f(.06, .06, .06, &textModel);
	
	GLuint tp_ul = glGetUniformLocation(textProg->id, "mProj");
	GLuint tm_ul = glGetUniformLocation(textProg->id, "mModel");
	GLuint ts_ul = glGetUniformLocation(textProg->id, "fontTex");
	
	glUniformMatrix4fv(tp_ul, 1, GL_FALSE, textProj.m);
	glUniformMatrix4fv(tm_ul, 1, GL_FALSE, textModel.m);
	glexit("text matrix uniforms");

	glDisable(GL_CULL_FACE);
	
	glActiveTexture(GL_TEXTURE0);
	glexit("active texture");

	glUniform1i(ts_ul, 0);
	glexit("text sampler uniform");
	glBindTexture(GL_TEXTURE_2D, arial->textureID);
	glexit("bind texture");
	
	
	glBindVertexArray(strRI->vao);
	glexit("text vao bind");
	
	glBindBuffer(GL_ARRAY_BUFFER, strRI->vbo);
	glexit("text vbo bind");
	glDrawArrays(GL_TRIANGLES, 0, strRI->vertexCnt);
	glexit("text drawing");
	
	
	glXSwapBuffers(xs->display, xs->clientWin);
}








void drawPatch() {
	
	glerr("pre vao bind");
	glBindVertexArray(vao);
	glerr("vao bind");
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glDrawArrays(GL_PATCHES, 0, 4);
	glerr("drawing");
}


void initPatch() {
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f, 
		1.0f, 1.0f, 0.0f, 
		1.0f, -1.0f, 0.0f, 
	};

	glPatchParameteri(GL_PATCH_VERTICES, 4);
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glerr("buffering data");
// 	glBindVertexArray(vbo);
	

	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
	glerr("vertex attrib ptr");

	
	
}





