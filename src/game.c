
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
#include "config.h"
#include "objloader.h"
#include "shader.h"
#include "texture.h"
#include "window.h"
#include "staticMesh.h"
#include "road.h"
#include "map.h"
#include "scene.h"
#include "game.h"
#include "emitter.h"


GLuint proj_ul, view_ul, model_ul;

Matrix mProj, mView, mModel;

float zoom;


// temp shit
TextRes* arial;
ShaderProgram* textProg;
Matrix textProj, textModel;
TextRenderInfo* strRI;
Texture* cnoise;
Emitter* dust;

StaticMesh* testmesh;

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




void setupFBOs(GameState* gs, int resized) {
	int ww = gs->screen.wh.x;
	int wh = gs->screen.wh.y;
	
	if(gs->fboTextures) {
		destroyFBOTextures(gs->fboTextures);
		free(gs->fboTextures);
	}
	
	// backing textures
	FBOTexConfig texcfg[] = {
		{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
		{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE},
		{GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE},
		{GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT},
		{0,0,0}
	};
	
	GLuint* texids = initFBOTextures(ww, wh, texcfg);
	
	gs->diffuseTexBuffer = texids[0];
	gs->normalTexBuffer = texids[1];
	gs->selectionTexBuffer = texids[2];
	gs->depthTexBuffer = texids[3];
	
	printf("New Main Depth: %d \n", texids[3]);
	
	// main gbuffer setup
	if(gs->gbuf.fb) { // evil abstraction breaking. meh.
		destroyFBO(&gs->gbuf);
	}
	
	FBOConfig gbufConf[] = {
		{GL_COLOR_ATTACHMENT0, gs->diffuseTexBuffer },
		{GL_COLOR_ATTACHMENT1, gs->normalTexBuffer },
		{GL_COLOR_ATTACHMENT2, gs->selectionTexBuffer },
		{GL_DEPTH_ATTACHMENT, gs->depthTexBuffer },
		{0,0}
	};
	
	initFBO(&gs->gbuf, gbufConf);
	
	// decal pass framebufer
	FBOConfig decalConf[] = {
		{GL_COLOR_ATTACHMENT0, gs->diffuseTexBuffer },
		{GL_COLOR_ATTACHMENT1, gs->normalTexBuffer },
		{GL_DEPTH_ATTACHMENT,  gs->depthTexBuffer },
		{0,0}
	};
	// depth buffer is also bound as a texture but disabled for writing
	
	initFBO(&gs->decalbuf, decalConf);
	
	
	
}

void initGame(XStuff* xs, GameState* gs) {
	int ww, wh;
	
	glerr("left over error on game init");
	
	gs->hasMoved = 1;
	gs->lastSelectionFrame = 0;
	gs->frameCount = 0;
	
	gs->activeTool = 0;
	
	gs->debugMode = 0;
	gs->sunSpeed = 0;
	
	gs->nearClipPlane = 20;
	gs->farClipPlane = 1700;

	
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
	
	setupFBOs(gs, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	
	printf("diffuse2: %d\n",gs->diffuseTexBuffer);
	// set up the Geometry Buffer

	
	shadingProg = loadCombinedProgram("shading");
	
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sDiffuse"), 0);
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sNormals"), 1);
	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sDepth"), 2);
 	glProgramUniform1i(shadingProg->id, glGetUniformLocation(shadingProg->id, "sSelection"), 3);
	
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
	getPrintGLEnum(GL_MAX_VERTEX_ATTRIBS, "meh");
	
	
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

	gs->zoom = -760.0;
	gs->direction = 0.0f;
	gs->lookCenter.x = 128;
	gs->lookCenter.y = 128;
	
	
	// initialize all those magic globals
	initMap(&gs->map);
	
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
	
	initStaticMeshes();
	initRoads();
	
	OBJContents cube;
	loadOBJFile("assets/models/untitled.obj", 0, &cube);
	//Mesh* cubem = OBJtoMesh(&cube);
	testmesh = StaticMeshFromOBJ(&cube);
	

	initEmitters();
	
	dust = makeEmitter();
	EmitterInstance dust_instance = {
		.pos = {0,0,150},
		.scale = 10,
		.start_time = 0,
		.lifespan = 1<<15
	};
	
	emitterAddInstance(dust, &dust_instance);
	
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

double timeSince(double past) {
	double now = getCurrentTime();
	return now - past;
}


void preFrame(GameState* gs) {
	
	// update timers
	char frameCounterBuf[128];
	unsigned int fpsColors[] = {0xeeeeeeff, 6, 0xeeee22ff, INT_MAX};
	
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
		
		snprintf(frameCounterBuf, 128, "dtime:  %.2fms", gs->perfTimes.draw * 1000);
		
		//printf("--->%s\n", frameCounterBuf);
		
		updateText(strRI, frameCounterBuf, -1, fpsColors);
		
		lastPoint = now;
	}
}


void postFrame(GameState* gs) {
	
	double now;
	
	now = getCurrentTime();
	
	gs->perfTimes.draw = now - gs->frameTime;
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
		gs->hasMoved = 1;
	}
	if(is->keyState[39] & IS_KEYDOWN) {
		gs->direction -= rotateSpeed;
		gs->hasMoved = 1;
	}
	// keep rotation in [0,F_2PI)
	gs->direction = fmodf(F_2PI + gs->direction, F_2PI);
	
	// zoom
	if(is->keyState[52] & IS_KEYDOWN) {
		gs->zoom += keyZoom;
 		gs->zoom = fmin(gs->zoom, -10.0);
		gs->hasMoved = 1;
	}
	if(is->keyState[53] & IS_KEYDOWN) {
		gs->zoom -= keyZoom;
		gs->hasMoved = 1;
	}
	if(is->clickButton == 4) {
		gs->zoom += mouseZoom;
 		gs->zoom = fmin(gs->zoom, -10.0);
		gs->hasMoved = 1;
	}
	if(is->clickButton == 5) {
		gs->zoom -= mouseZoom;
		gs->hasMoved = 1;
	}

	// movement
	Vector move = {
		.x = moveSpeed * sin(F_PI - gs->direction),
		.y = moveSpeed * cos(F_PI - gs->direction),
		.z = 0.0f
	};
	
	if(is->keyState[111] & IS_KEYDOWN) {
		vAdd(&gs->lookCenter, &move, &gs->lookCenter);
		gs->hasMoved = 1;
	}
	if(is->keyState[116] & IS_KEYDOWN) {
		vSub(&gs->lookCenter, &move, &gs->lookCenter);
		gs->hasMoved = 1;
	}
	
	// flip x and y to get ccw normal, using move.z as the temp
	move.z = move.x;
	move.x = -move.y;
	move.y = move.z;
	move.z = 0.0f;
	
	if(is->keyState[113] & IS_KEYDOWN) {
		vSub(&gs->lookCenter, &move, &gs->lookCenter);
		gs->hasMoved = 1;
	}
	if(is->keyState[114] & IS_KEYDOWN) {
		vAdd(&gs->lookCenter, &move, &gs->lookCenter);
		gs->hasMoved = 1;
	}
	
	// these don't stimulate a selection pass since they are debug tools
	if(is->keyState[110] & IS_KEYDOWN) {
		gs->nearClipPlane += 50 * te;
		printf("near: %f, far: %f\n", gs->nearClipPlane, gs->farClipPlane);
	}
	if(is->keyState[115] & IS_KEYDOWN) {
		gs->nearClipPlane -= 50 * te;
		printf("near: %f, far: %f\n", gs->nearClipPlane, gs->farClipPlane);
	}
	if(is->keyState[112] & IS_KEYDOWN) {
		gs->farClipPlane += 250 * te;
		printf("near: %f, far: %f\n", gs->nearClipPlane, gs->farClipPlane);
	}
	if(is->keyState[117] & IS_KEYDOWN) {
		gs->farClipPlane -= 250 * te;
		printf("near: %f, far: %f\n", gs->nearClipPlane, gs->farClipPlane);
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


// actually the selection pass
void depthPrepass(XStuff* xs, GameState* gs, InputState* is) {
	
	// draw UI
	renderUIPicking(xs, gs);
	
	
	
	//updateView(xs, gs, is);
	

	// draw terrain
// 	drawTerrainBlockDepth(&gs->map, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj));
	drawTerrainDepth(&gs->map, msGetTop(&gs->view), msGetTop(&gs->proj), &gs->screen.wh);
	
	//msPop(&gs->view);
	//msPop(&gs->proj);
	
}


void updateView(XStuff* xs, GameState* gs, InputState* is) {
		
	gs->sunNormal.x = cos(gs->frameTime * gs->sunSpeed);
	gs->sunNormal.y = sin(gs->frameTime * gs->sunSpeed);
	gs->sunNormal.z = 0.0;
	
	msPush(&gs->proj);
	msPerspective(60, gs->screen.aspect, gs->nearClipPlane, gs->farClipPlane, &gs->proj);
	
	
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
	
}

void cleanUpView(XStuff* xs, GameState* gs, InputState* is) {
	msPop(&gs->view);
	msPop(&gs->proj);
	
}

void renderFrame(XStuff* xs, GameState* gs, InputState* is) {
	
	//mModel = IDENT_MATRIX;
	
	Vector2 c2;
	
	c2.x = 300; //cursorp.x;
	c2.y = 300; //cursorp.z;
	
	//updateView(xs, gs, is);
	
	// draw terrain
// 	drawTerrainBlock(&gs->map, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos);
	drawTerrain(&gs->map, msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos, &gs->screen.wh);
	
	renderMarker(gs, 0,0);

	//drawStaticMesh(testmesh, msGetTop(&gs->view), msGetTop(&gs->proj));
	

	
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

void renderDecals(XStuff* xs, GameState* gs, InputState* is) {
	/*
	glActiveTexture(GL_TEXTURE0 + 8);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, gs->depthTexBuffer);
	glUniform1i(glGetUniformLocation(shadingProg->id, "sDepth"), 8);
	
	*/
	
	drawTerrainRoads(gs->depthTexBuffer, &gs->map, msGetTop(&gs->view), msGetTop(&gs->proj), &gs->cursorPos, &gs->screen.wh);
	
	glexit("render decals");
}


void shadingPass(GameState* gs) {
	
	Matrix world;
	
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

	
	glUniform1i(glGetUniformLocation(shadingProg->id, "debugMode"), gs->debugMode);
	glUniform2f(glGetUniformLocation(shadingProg->id, "clipPlanes"), gs->nearClipPlane, gs->farClipPlane);
	
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
	
	unsigned char rgb[4];
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
		GL_UNSIGNED_BYTE,
		&rgb);
	glexit("read selection");
	
	gs->cursorTilePos.x = rgb[0];
	gs->cursorTilePos.y = rgb[1];
	gs->cursorTilePos.z = rgb[2];
	
	struct sGL_RG8* off = &gs->map.offsetData[(int)gs->cursorTilePos.z]; 
	
	gs->cursorPos.x = (off->x * 256.0) + gs->cursorTilePos.x;
	gs->cursorPos.y = (off->y * 256.0) + gs->cursorTilePos.y;
	
	//printf("pos: x: %d, y:%d \n", (int)gs->cursorPos.x, (int)gs->cursorPos.y);
	/*
 	printf("mx: %d, my: %d, x: %d, y: %d, z: %d\n", 
		   (int)is->cursorPosPixels.x, 
		   (int)is->cursorPosPixels.y,  
		   rgb[0], rgb[1], rgb[2]);
	*/
	
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
		
		// TODO: destroy all the textures too
		
		printf("screen 0 resized\n");
		
		viewWH.x = xs->winAttr.width;
		viewWH.y = xs->winAttr.height;
		
		gs->screen.wh.x = (float)xs->winAttr.width;
		gs->screen.wh.y = (float)xs->winAttr.height;
		
		gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
		
		gs->screen.resized = 1;
		
		setupFBOs(gs, 1);
		
		printf("diffuse2: %d\n",gs->diffuseTexBuffer);
	}
}

#define PF_START(x) gs->perfTimes.x = getCurrentTime()
#define PF_STOP(x) gs->perfTimes.x = timeSince(gs->perfTimes.x)

void gameLoop(XStuff* xs, GameState* gs, InputState* is) {
	gs->frameCount++;
	
	checkResize(xs,gs);
	
		PF_START(preframe);
	preFrame(gs);
		PF_STOP(preframe);
	
	handleInput(gs, is);
	
	//setUpView(gs);
	updateView(xs, gs, is);
	
	// update world state
	
	if(gs->hasMoved && gs->lastSelectionFrame < gs->frameCount - 8) {
		printf("doing selection pass %d\n", gs->frameCount);
		gs->hasMoved = 0;
		gs->lastSelectionFrame = gs->frameCount; 
		
		// really just the selection pass
			PF_START(selection);
		glDepthFunc(GL_LESS);
		glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		depthPrepass(xs, gs, is);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gs->gbuf.fb);
		
		checkCursor(gs, is);
		
		glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
			PF_STOP(selection);
			
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, gs->gbuf.fb);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
		
		PF_START(draw);

	
	
	// clear color buffer for actual rendering
	//glClear(GL_COLOR_BUFFER_BIT);
	glerr("pre shader create 1d");
	glDepthFunc(GL_LEQUAL);
	glerr("pre shader create e");
	
	renderFrame(xs, gs, is);
	
		PF_STOP(draw);
		PF_START(decal);

	// decals
	glBindFramebuffer(GL_FRAMEBUFFER, gs->decalbuf.fb);
	
	glDepthMask(GL_FALSE); // disable depth writes for decals
	renderDecals(xs, gs, is);
	glDepthMask(GL_TRUE);
		
		PF_STOP(decal);
	
	
	cleanUpView(xs, gs, is);
	
	// draw to the screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	shadingPass(gs);
	
	renderUI(xs, gs);
	
	gs->screen.resized = 0;

	postFrame(gs);

	
	glXSwapBuffers(xs->display, xs->clientWin);

	
}







