
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

double nearPlane = 20;
double farPlane = 1700;


// temp shit
TextRes* arial;
ShaderProgram* textProg;
Matrix textProj, textModel;
TextRenderInfo* strRI;
Texture* cnoise;

MapBlock* map;
// TerrainBlock* terrain;

GLuint fsQuadVAO, fsQuadVBO;
ShaderProgram* shadingProg;

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


#define getPrintGLEnum(e, m) _getPrintGLEnumMin(e, #e, m)

int _getPrintGLEnumMin(GLenum e, char* name, char* message) {
	GLint i;
	
	glGetIntegerv(e, &i);
	printf("%s: %d\n", name, i);
	
	return i;
}




GLuint initTexBuffer(int w, int h, GLenum internalType, GLenum format, GLenum size) {
	GLuint id;
	
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
		glexit(" -- tex buffer creation");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glexit("pre tex buffer creation");
	glTexImage2D(GL_TEXTURE_2D, 0, internalType, w, h, 0, format, size, NULL);
	glexit("tex buffer creation");
	
	return id;
}
GLuint initTexBufferRGBA(int w, int h) {
	return initTexBuffer(w, h, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
}
GLuint initTexBufferDepth(int w, int h) {
	//GLuint id = initTexBuffer(w, h, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);

		GLuint id;
	
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
		glexit(" -- tex buffer creation");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glexit("a pre tex buffer creation");
	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_RED);
	
// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glexit("b pre tex buffer creation");
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, w, h, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
 	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glexit("tex buffer creation");
	

	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	return id;
}

void initGame(XStuff* xs, GameState* gs) {
	int ww, wh;
	
	gs->debugMode = 0;
	
	glerr("left over error on game init");
	
	
	gs->viewW = ww = xs->winAttr.width;
	gs->viewH = wh = xs->winAttr.height;

	printf("w: %d, h: %d\n", ww, wh);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
glexit("here");


	
	printf("0\n");
	gs->diffuseTexBuffer = initTexBufferRGBA(ww, wh); 
	printf("1\n");
	gs->normalTexBuffer = initTexBufferRGBA(ww, wh); 
	printf("2\n");
	gs->selectionTexBuffer = initTexBuffer(ww, wh, GL_RGB16I, GL_RGB_INTEGER, GL_SHORT);
	printf("2\n");
	gs->depthTexBuffer = initTexBufferDepth(ww, wh); 

	glBindTexture(GL_TEXTURE_2D, 0);

	
	glGenFramebuffers(1, &gs->framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gs->framebuffer);
	glexit("fbo creation");
	
	// The depth buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gs->diffuseTexBuffer, 0); 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gs->normalTexBuffer, 0);  
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gs->selectionTexBuffer, 0);  
 	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gs->depthTexBuffer, 0);  
	glexit("fb tex2d");
	
//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, gs->depthTexBuffer, 0);
	
	GLenum DrawBuffers[] = {
		GL_COLOR_ATTACHMENT0, 
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};
	glDrawBuffers(3, DrawBuffers);
	glexit("drawbuffers");
	
	GLenum status;
	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE) {
		printf("fbo status invalid\n");
		exit(1);
	}
	printf("FBO created.\n");
	

	shadingProg = loadProgram("shading", "shading", NULL, NULL, NULL);
	initFSQuad();
	
	// check some prerequisites
// 	GLint MaxDrawBuffers = 0;
// 	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MaxDrawBuffers);
// 	printf("MAX_DRAW_BUFFERS: %d\n", MaxDrawBuffers);
//  	if(MaxDrawBuffers < 4) {
// 		fprintf(stderr, "FATAL: GL_MAX_DRAW_BUFFERS is too low: %d. Minimum required value is 4.\n", MaxDrawBuffers);
// 		exit(3);
// 	};

	getPrintGLEnum(GL_MAX_COLOR_ATTACHMENTS, "meh");
	getPrintGLEnum(GL_MAX_DRAW_BUFFERS, "meh");
	getPrintGLEnum(GL_MAX_FRAMEBUFFER_WIDTH, "meh");
	getPrintGLEnum(GL_MAX_FRAMEBUFFER_HEIGHT, "meh");
	getPrintGLEnum(GL_MAX_FRAMEBUFFER_LAYERS, "meh");
	getPrintGLEnum(GL_MAX_FRAMEBUFFER_SAMPLES, "meh");
	
	
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
// 	msPerspective(60, 1.0, 01000.0f, 100000.0f, proj);
// 		msOrtho(0, 1, 0, 1, .01, 100000, proj);

	gs->zoom = -960.0;
	gs->direction = 0.0;
	angle = 0.2;
	gs->lookCenter.x = 512;
	gs->lookCenter.y = 512;
	
	
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
	if(is->clickButton == 4) {
		gs->zoom +=  50;
 		gs->zoom = fmin(gs->zoom, -10.0);
	}
	if(is->clickButton == 5) {
		gs->zoom -=  50; 
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
	
	if(is->keyState[110] & IS_KEYDOWN) {
		nearPlane += 50 * te;
		printf("near: %f, far: %f\n", nearPlane, farPlane);
	}
	if(is->keyState[115] & IS_KEYDOWN) {
		nearPlane -= 50 * te; 
		printf("near: %f, far: %f\n", nearPlane, farPlane);
	}
	if(is->keyState[112] & IS_KEYDOWN) {
		farPlane += 250 * te;
		printf("near: %f, far: %f\n", nearPlane, farPlane);
	}
	if(is->keyState[117] & IS_KEYDOWN) {
		farPlane -= 250 * te; 
		printf("near: %f, far: %f\n", nearPlane, farPlane);
	}
	
	static lastChange = 0;
	if(is->keyState[119] & IS_KEYDOWN) {
		if(gs->frameTime > lastChange + 1) {
			gs->debugMode = (gs->debugMode + 1) % 5; 
			lastChange = gs->frameTime;
		}
	}
	
}


void setUpView(GameState* gs) {
	
	
}

void renderFrame(XStuff* xs, GameState* gs, InputState* is) {
	
	//mModel = IDENT_MATRIX;
	
	
	 
	gs->sunNormal.x = cos(gs->frameTime * .5);
	gs->sunNormal.y = sin(gs->frameTime * .5);
	gs->sunNormal.z = 0.0;
	
	
	angle = (rot * 3.14159265358979) / 180 ;
	//mScale3f(10, 10, 10, &mModel);
	//mRot3f(0, 1, 0, angle, &mModel);
	msPush(&gs->proj);
	msPerspective(60, 1.0, nearPlane, farPlane, &gs->proj);

	
	msPush(&gs->view);
	
	
	
	// order matters! don't mess with this.
	msTrans3f(0, -1, gs->zoom, &gs->view);
	msRot3f(1, 0, 0, 3.1415/6, &gs->view);
	msRot3f(0,1,0, angle, &gs->view);
	msTrans3f(-gs->lookCenter.x + 512, 0, -gs->lookCenter.y + 512, &gs->view);
	// TODO: fix coordinates

	
 	msPush(&gs->model);
	
	// move it to the middle of the screen
 	msTrans3f(-.5, 0, -.5, &gs->model);
	
	// y-up to z-up rotation
	msRot3f(1, 0, 0, 3.1415/2, &gs->model);
	
	

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
	drawTerrainBlock(gs->terrain, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos);
	
	
	msPop(&gs->model);
	msPop(&gs->view);
	msPop(&gs->proj);
	
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
	
	
	
}

void shadingPass(GameState* gs) {
	
	Matrix world;
	
	world = IDENT_MATRIX;
	
	glUseProgram(shadingProg->id);
	glexit("shading prog");

	glActiveTexture(GL_TEXTURE0 + 6);
	glexit("shading tex 1");
	glBindTexture(GL_TEXTURE_2D, gs->diffuseTexBuffer);
	glexit("shading tex 2");
	glActiveTexture(GL_TEXTURE0 + 7);
	glexit("shading tex 3");
	glBindTexture(GL_TEXTURE_2D, gs->normalTexBuffer);
	glexit("shading tex 4");
	glActiveTexture(GL_TEXTURE0 + 8);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, gs->depthTexBuffer);
	glexit("shading tex 6");
	glActiveTexture(GL_TEXTURE0 + 9);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, gs->selectionTexBuffer);
	glexit("shading tex 6");
	
	glUniform1i(glGetUniformLocation(shadingProg->id, "debugMode"), gs->debugMode);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sDiffuse"), 6);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sNormals"), 7);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sDepth"), 8);
 	glUniform1i(glGetUniformLocation(shadingProg->id, "sSelection"), 9);
	glexit("shading samplers");
	
	glUniformMatrix4fv(glGetUniformLocation(shadingProg->id, "world"), 1, GL_FALSE, world.m);
	glexit("shading world");

	glUniform3fv(glGetUniformLocation(shadingProg->id, "sunNormal"), 1, &gs->sunNormal);
	
	
	drawFSQuad();
	glexit("post quad draw");
}

void checkCursor(GameState* gs, InputState* is) {
	
	unsigned short rgb[4];
	
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glexit("selection buff");
	glReadPixels(
		is->cursorPosPixels.x,
		is->cursorPosPixels.y,
		1,
		1,
		GL_RGB_INTEGER,
		GL_SHORT,
		&rgb);
	glexit("read selection");
	
	gs->cursorPos.x = rgb[0];
	gs->cursorPos.y = rgb[1];
	
// 	printf("mx: %d, my: %d, x: %d, y: %d\n", (int)is->cursorPosPixels.x, (int)is->cursorPosPixels.y, rgb[0], rgb[1]);
	
	if(is->clickButton == 3) {
		gs->lookCenter.x = gs->cursorPos.x;
		gs->lookCenter.y = gs->cursorPos.y;
	}
	
}


void gameLoop(XStuff* xs, GameState* gs, InputState* is) {
	
	preFrame(gs);
	
	handleInput(gs, is);
	
	setUpView(gs);
	
	// update world state
	
	// draw to the g-buffer
	glBindFramebuffer(GL_FRAMEBUFFER, gs->framebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	renderFrame(xs, gs, is);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gs->framebuffer);
	
	checkCursor(gs, is);
	
	// draw to the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	shadingPass(gs);
	
	
	glXSwapBuffers(xs->display, xs->clientWin);

}







