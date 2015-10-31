
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

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
// TerrainBlock* terrain;

void initPatch();
void drawPatch();

void initGame(XStuff* xs, GameState* gs) {

	
	glerr("left over error on game init");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	
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
	
	
	// initialize all those magic globals
	initTerrain();
	

	
	gs->terrain = allocTerrainBlock(0, 0);
	updateTerrainTexture(gs->terrain);
	

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
	
	// CLOCK_MONOTONIC_RAW is linux-specific. 
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	
	now = (double)ts.tv_sec + ((double)ts.tv_nsec / 1000000000.0);
	if(offset == 0) offset = now;
	
	return now - offset;
}


float rot = 0;



void preFrame(GameState* gs) {
	
	// update timers
	char frameCounterBuf[128];
	unsigned int fpsColors[] = {0xeeeeeeff, 4, 0xeeee22ff, INT_MAX};
	
	static int frameCounter = 0;
	static double last_frame = 0;
	static double lastPoint = 0;
	
	double now;
	
	gs->frameTime = now = getCurrentTime();

	if (last_frame == 0)
		last_frame = now;
	
	gs->frameSpan = (double)(now - last_frame);
	last_frame = now;
	
	frameCounter = (frameCounter + 1) % 60;
	
	if(lastPoint == 0.0f) lastPoint = gs->frameTime;
	if(frameCounter == 0) {
		float fps = 60.0f / (gs->frameTime - lastPoint);
		
		snprintf(frameCounterBuf, 128, "FPS:  %.2f", fps);
		
		//printf("--->%s\n", frameCounterBuf);
		
		updateText(strRI, frameCounterBuf, -1, fpsColors);
		
		lastPoint = now;
	}
}



void handleInput(GameState* gs, InputState* is) {
	double te = gs->frameSpan;
	
	
	// look direction
	if(is->keyState[38] & IS_KEYDOWN) {
		rot +=  20.8 * te;
	}
	if(is->keyState[39] & IS_KEYDOWN) {
		rot -=  20.8 * te;
	}
	
	// zoom
	if(is->keyState[52] & IS_KEYDOWN) {
		gs->zoom +=  150 * te;
 		gs->zoom = fmin(gs->zoom, -10.0);
	}
	if(is->keyState[53] & IS_KEYDOWN) {
		gs->zoom -=  150 * te; 
	}

	// movement
	if(is->keyState[113] & IS_KEYDOWN) {
		gs->lookCenter.x +=  250 * te; 
	}
	if(is->keyState[114] & IS_KEYDOWN) {
		gs->lookCenter.x -=  250 * te; 
	}
	
	if(is->keyState[111] & IS_KEYDOWN) {
		gs->lookCenter.y +=  250 * te;
	}
	if(is->keyState[116] & IS_KEYDOWN) {
		gs->lookCenter.y -=  250 * te; 
	}
	
}


void setUpView(GameState* gs) {
	
	
}

void renderFrame(XStuff* xs, GameState* gs, InputState* is) {
	
	//mModel = IDENT_MATRIX;
	

	
	
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
	
	

	// calculate cursor position
	Vector cursorp;
	Vector eyeCoord;
	Vector worldCoord;
	Matrix p, invp, invv;
	
	// device space (-1:1)
	Vector devCoord;
	devCoord.x = 0.50;
	devCoord.y = 0.50;
	devCoord.z = -1.0;
	
	// eye space
	mInverse(msGetTop(&gs->proj), &invp);
	vMatrixMul(&devCoord, &invp, &eyeCoord);
	vNorm(&eyeCoord, &eyeCoord);
	
	// world space
	mInverse(msGetTop(&gs->view), &invv);
	vMatrixMul(&eyeCoord, &invv, &worldCoord);
	vNorm(&worldCoord, &worldCoord);
	
	
	
	
// 	mFastMul(msGetTop(&gs->view), msGetTop(&gs->proj), &mp);
	
// 	mInverse(&mp, &invmp);
	
	
	
	
// 	vMatrixMul(&devCoord, &invmp, &cursorp);
	
	//printf("(%f, %f, %f)\n", worldCoord.x, worldCoord.y, worldCoord.z);
	
	Vector2 c2;
	
	c2.x = 300; //cursorp.x;
	c2.y = 300; //cursorp.z;
	
	
	
	// draw terrain
	drawTerrainBlock(gs->terrain, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj), (Vector2*)&c2);
	
	
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



void gameLoop(XStuff* xs, GameState* gs, InputState* is) {
	
	preFrame(gs);
	
	handleInput(gs, is);
	
	setUpView(gs);
	
	// update world state
	
	renderFrame(xs, gs, is);
	

}







