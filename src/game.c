
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

float zoom;

double nearPlane = 20;
double farPlane = 1700;


// temp shit
TextRes* arial;
ShaderProgram* textProg;
Matrix textProj, textModel;
TextRenderInfo* strRI;
Texture* cnoise;

// MapBlock* map;
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




GLuint initTexBuffer(GLuint id, int w, int h, GLenum internalType, GLenum format, GLenum size) {
	if(!id) {
		glGenTextures(1, &id);
	}
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
GLuint initTexBufferRGBA(GLuint id, int w, int h) {
	return initTexBuffer(id, w, h, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
}
GLuint initTexBufferDepth(GLuint id, int w, int h) {
	//GLuint id = initTexBuffer(w, h, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
	
	if(!id) {
		glGenTextures(1, &id);
	}
	
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

void initTexBuffers(GameState* gs, int resized) {
	int ww = gs->screen.wh.x;
	int wh = gs->screen.wh.y;
	
	if(!resized) {
		gs->diffuseTexBuffer = 0;
		gs->normalTexBuffer = 0;
		gs->selectionTexBuffer = 0;
		gs->depthTexBuffer = 0;
	}
	
	printf("tex 0\n");
	gs->diffuseTexBuffer = initTexBufferRGBA(gs->diffuseTexBuffer, ww, wh);
	printf("tex 1\n");
	gs->normalTexBuffer = initTexBufferRGBA(gs->normalTexBuffer, ww, wh);
	printf("tex 2\n");
	gs->selectionTexBuffer = initTexBuffer(gs->selectionTexBuffer, ww, wh, GL_RGB16I, GL_RGB_INTEGER, GL_SHORT);
	printf("tex 3\n");
	gs->depthTexBuffer = initTexBufferDepth(gs->depthTexBuffer, ww, wh);
	printf("tex 4\n");
}

void initGame(XStuff* xs, GameState* gs) {
	int ww, wh;
	
	glerr("left over error on game init");
	
	
	gs->activeTool = 0;
	
	gs->debugMode = 0;
	gs->sunSpeed = 0;
	
	
	ww = xs->winAttr.width;
	wh = xs->winAttr.height;
	
	gs->screen.wh.x = (float)ww;
	gs->screen.wh.y = (float)wh;
	
	gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
	gs->screen.resized = 0;
	
	
	printf("w: %d, h: %d\n", ww, wh);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	
	initTexBuffers(gs, 0);
	
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
	

	shadingProg = loadCombinedProgram("shading");
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
	getPrintGLEnum(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "meh");
	getPrintGLEnum(GL_MAX_ARRAY_TEXTURE_LAYERS, "meh");
	getPrintGLEnum(GL_MAX_SAMPLES, "meh");
	
	
	// set up matrix stacks
	MatrixStack* view, *proj;
	
	view = &gs->view;
	proj = &gs->proj;
	
	msAlloc(2, view);
	msAlloc(2, proj);

	msIdent(view);
	msIdent(proj);
	

	
	
	// perspective matrix, pretty standard
// 	msPerspective(60, 1.0, 01000.0f, 100000.0f, proj);
// 		msOrtho(0, 1, 0, 1, .01, 100000, proj);

	gs->zoom = -960.0;
	gs->direction = 0.0f;
	gs->lookCenter.x = 512;
	gs->lookCenter.y = 512;
	
	
	// initialize all those magic globals
	initMap(&gs->map);
	updateTerrainTexture(gs->map.tb);
	
	initUI(gs);
	initMarker();
	
	// text rendering stuff
	arial = LoadFont("Arial", 64, NULL);
	glerr("clearing before text program load");
	textProg = loadCombinedProgram("text");
	
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
	
	float moveSpeed = gs->settings.keyScroll * te; // should load from config
	float rotateSpeed = gs->settings.keyRotate * te; // 20.8 degrees
	float keyZoom = gs->settings.keyZoom * te;
	float mouseZoom = gs->settings.mouseZoom * te;
	
	if(is->clickButton == 1) {
		/*
		flattenArea(gs->map.tb,
			gs->cursorPos.x - 5,
			gs->cursorPos.y - 5,
			gs->cursorPos.x + 5,
			gs->cursorPos.y + 5
		);
		
		setZone(&gs->map,
			gs->cursorPos.x - 5,
			gs->cursorPos.y - 5,
			gs->cursorPos.x + 5,
			gs->cursorPos.y + 5,
			gs->activeTool + 1
		);
		
		
		checkMapDirty(&gs->map);
		*/
	}
	
	if(is->buttonDown == 1) {
		gs->mouseDownPos.x = gs->cursorPos.x;
		gs->mouseDownPos.y = gs->cursorPos.y;
		printf("start dragging at (%d,%d)\n", (int)gs->cursorPos.x, (int)gs->cursorPos.y);
	}
	if(is->buttonUp == 1) {
		//vCopy(&gs->cursorPos, &gs->mouseDownPos);
		printf("stopped dragging at (%d,%d)\n", (int)gs->cursorPos.x, (int)gs->cursorPos.y);
	}
	
	if(is->clickButton == 2) {
		gs->activeTool = (gs->activeTool + 1) % 3;
	}
	
	// look direction
	if(is->keyState[38] & IS_KEYDOWN) {
		gs->direction += rotateSpeed;
	}
	if(is->keyState[39] & IS_KEYDOWN) {
		gs->direction -= rotateSpeed;
	}
	// keep rotation in [0,F_2PI)
	gs->direction = fmodf(F_2PI + gs->direction, F_2PI);
	
	// zoom
	if(is->keyState[52] & IS_KEYDOWN) {
		gs->zoom += keyZoom;
 		gs->zoom = fmin(gs->zoom, -10.0);
	}
	if(is->keyState[53] & IS_KEYDOWN) {
		gs->zoom -= keyZoom;
	}
	if(is->clickButton == 4) {
		gs->zoom += mouseZoom;
 		gs->zoom = fmin(gs->zoom, -10.0);
	}
	if(is->clickButton == 5) {
		gs->zoom -= mouseZoom;
	}

	// movement
	Vector move = {
		.x = moveSpeed * sin(F_PI - gs->direction),
		.y = moveSpeed * cos(F_PI - gs->direction),
		.z = 0.0f
	};
	
	if(is->keyState[111] & IS_KEYDOWN) {
		vAdd(&gs->lookCenter, &move, &gs->lookCenter);
	}
	if(is->keyState[116] & IS_KEYDOWN) {
		vSub(&gs->lookCenter, &move, &gs->lookCenter);
	}
	
	// flip x and y to get ccw normal, using move.z as the temp
	move.z = move.x;
	move.x = -move.y;
	move.y = move.z;
	move.z = 0.0f;
	
	if(is->keyState[113] & IS_KEYDOWN) {
		vSub(&gs->lookCenter, &move, &gs->lookCenter);
	}
	if(is->keyState[114] & IS_KEYDOWN) {
		vAdd(&gs->lookCenter, &move, &gs->lookCenter);
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


void setGameSettings(GameSettings* g, UserConfig* u) {
	
	const float rotateFactor = 0.7260f;
	const float scrollFactor = 300.0f;
	const float zoomFactor = 600.0f;
	
	g->keyRotate = rotateFactor * fclampNorm(u->keyRotateSensitivity);
	g->keyScroll = scrollFactor * fclampNorm(u->keyScrollSensitivity);
	g->keyZoom = zoomFactor * fclampNorm(u->keyZoomSensitivity);
	
	g->mouseRotate = rotateFactor * fclampNorm(u->mouseRotateSensitivity);
	g->mouseScroll = scrollFactor * fclampNorm(u->mouseScrollSensitivity);
	g->mouseZoom = 4 * zoomFactor * fclampNorm(u->mouseZoomSensitivity);
	
	printf("keyRotate %.3f\n", g->keyRotate);
	printf("keyScroll %.3f\n", g->keyScroll);
	printf("keyZoom %.3f\n", g->keyZoom);
	printf("mouseRotate %.3f\n", g->mouseRotate);
	printf("mouseScroll %.3f\n", g->mouseScroll);
	printf("mouseZoom %.3f\n", g->mouseZoom);
	
}


void setUpView(GameState* gs) {
	
	
}



void depthPrepass(XStuff* xs, GameState* gs, InputState* is) {
	
	// draw UI
	renderUIPicking(xs, gs);
	
	
	// draw terrain
	// TODO: factor all the math into the frame setup function
	//mScale3f(10, 10, 10, &mModel);
	//mRot3f(0, 1, 0, gs->direction, &mModel);
	msPush(&gs->proj);
	msPerspective(60, gs->screen.aspect, nearPlane, farPlane, &gs->proj);

	
	msPush(&gs->view);
	
	
	
	// order matters! don't mess with this.
	msTrans3f(0, -1, gs->zoom, &gs->view);
	msRot3f(1, 0, 0, F_PI / 6, &gs->view);
	msRot3f(0,1,0, gs->direction, &gs->view);
	msTrans3f(-gs->lookCenter.x, 0, -gs->lookCenter.y, &gs->view);
	
	// y-up to z-up rotation
	msRot3f(1, 0, 0, F_PI_2, &gs->view);
	msScale3f(1, 1, -1, &gs->view);


	
	
	// calculate cursor position
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

	// draw terrain
// 	drawTerrainBlockDepth(&gs->map, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj));
	drawTerrainDepth(&gs->map, msGetTop(&gs->view), msGetTop(&gs->proj), &gs->screen.wh);
	
	msPop(&gs->view);
	msPop(&gs->proj);
	
}

void renderFrame(XStuff* xs, GameState* gs, InputState* is) {
	
	//mModel = IDENT_MATRIX;
	
	
	 
	gs->sunNormal.x = cos(gs->frameTime * gs->sunSpeed);
	gs->sunNormal.y = sin(gs->frameTime * gs->sunSpeed);
	gs->sunNormal.z = 0.0;
	
	
	//mScale3f(10, 10, 10, &mModel);
	//mRot3f(0, 1, 0, gs->direction, &mModel);
	msPush(&gs->proj);
	msPerspective(60, gs->screen.aspect, nearPlane, farPlane, &gs->proj);

	
	msPush(&gs->view);
	
	

	// order matters! don't mess with this.
	msTrans3f(0, -1, gs->zoom, &gs->view);
	msRot3f(1, 0, 0, F_PI / 6, &gs->view);
	msRot3f(0,1,0, gs->direction, &gs->view);
	msTrans3f(-gs->lookCenter.x, 0, -gs->lookCenter.y, &gs->view);
	
	// y-up to z-up rotation
	msRot3f(1, 0, 0, F_PI_2, &gs->view);
	msScale3f(1, 1, -1, &gs->view);
	
	
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
// 	drawTerrainBlock(&gs->map, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos);
	drawTerrain(&gs->map, msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos, &gs->screen.wh);
	
	renderMarker(gs, 0,0);

	msPop(&gs->view);
	msPop(&gs->proj);
	
	glUseProgram(textProg->id);
	
	
	
	// text stuff
	textProj = IDENT_MATRIX;
	textModel = IDENT_MATRIX;
	
	mOrtho(0, gs->screen.aspect, 0, 1, -1, 100, &textProj);
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

	glUniform3fv(glGetUniformLocation(shadingProg->id, "sunNormal"), 1, (float*)&gs->sunNormal);
	
	if(gs->screen.resized) {
		glUniform2fv(glGetUniformLocation(shadingProg->id, "resolution"), 1, (float*)&gs->screen.wh);
	}
	
	drawFSQuad();
	glexit("post quad draw");
}

void checkCursor(GameState* gs, InputState* is) {
	
	unsigned short rgb[4];
	glexit("pre selection buff");
	
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glexit("selection buff");
	//printf("cursor pixels: %f, %f\n", is->cursorPosPixels.x, is->cursorPosPixels.y);
	glReadPixels(
		is->cursorPosPixels.x,
		is->cursorPosPixels.y,
		1,
		1,
		GL_RGB_INTEGER,
		GL_UNSIGNED_SHORT,
		&rgb);
	glexit("read selection");
	
	gs->cursorPos.x = rgb[0];
	gs->cursorPos.y = rgb[1];
	
// 	printf("mx: %d, my: %d, x: %d, y: %d\n", (int)is->cursorPosPixels.x, (int)is->cursorPosPixels.y, rgb[0], rgb[1]);
	
	if(is->clickButton == 3 && rgb[2] == 1) {
		gs->lookCenter.x = gs->cursorPos.x;
		gs->lookCenter.y = gs->cursorPos.y;
	}
	
}

Vector2i viewWH = {
	.x = 0,
	.y = 0
};
void checkResize(XStuff* xs, GameState* gs) {
	if(viewWH.x != xs->winAttr.width || viewWH.y != xs->winAttr.height) {
		printf("screen 0 resized\n");
		
		viewWH.x = xs->winAttr.width;
		viewWH.y = xs->winAttr.height;
		
		gs->screen.wh.x = (float)xs->winAttr.width;
		gs->screen.wh.y = (float)xs->winAttr.height;
		
		gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
		
		gs->screen.resized = 1;
		
		initTexBuffers(gs, 1);
	}
}


void gameLoop(XStuff* xs, GameState* gs, InputState* is) {
	
	checkResize(xs,gs);
	
	preFrame(gs);
	
	handleInput(gs, is);
	
	setUpView(gs);
	
	// update world state
	
	
	// depth and picking pre-pass
	glDepthFunc(GL_LESS);
	glBindFramebuffer(GL_FRAMEBUFFER, gs->framebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	depthPrepass(xs, gs, is);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gs->framebuffer);
	
	checkCursor(gs, is);
	
	glBindFramebuffer(GL_FRAMEBUFFER, gs->framebuffer);
	
	
	// clear color buffer for actual rendering
	//glClear(GL_COLOR_BUFFER_BIT);
	glerr("pre shader create 1d");
	glDepthFunc(GL_LEQUAL);
	glerr("pre shader create e");
	
	renderFrame(xs, gs, is);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gs->framebuffer);
	
	
	// draw to the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	shadingPass(gs);
	
	renderUI(xs, gs);
	
	gs->screen.resized = 0;
	
	glXSwapBuffers(xs->display, xs->clientWin);

}







