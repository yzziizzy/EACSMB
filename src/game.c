
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>



#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"
#include "text/text.h"
#include "c_json/json.h"
#include "json_gl.h"

#include "utilities.h"
#include "config.h"
#include "objloader.h"
#include "shader.h"
#include "texture.h"
#include "window.h"
#include "staticMesh.h"
#include "road.h"
#include "map.h"
#include "game.h"
#include "emitter.h"
#include "scene.h"
#include "gui.h"



GLuint proj_ul, view_ul, model_ul;

Matrix mProj, mView, mModel;

float zoom;


// temp shit
GUIText* gt, *gt_sel, *gt_emit;
GUIText* gtRenderMode;
GUIText* gtSelectionDisabled;


Texture* cnoise;
Emitter* dust;
RoadBlock* roads;




// MapBlock* map;
// TerrainBlock* terrain;



#define getPrintGLEnum(e, m) _getPrintGLEnumMin(e, #e, m)

int _getPrintGLEnumMin(GLenum e, char* name, char* message) {
	GLint i;
	
	glGetIntegerv(e, &i);
	printf("%s: %d\n", name, i);
	
	return i;
}


void initGame(XStuff* xs, GameState* gs) {
	int ww, wh;
	
	srand((unsigned int)time(NULL));
	
	json_gl_init_lookup();
	
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
	
	
	initUniformBuffers();
	
	uniformBuffer_init(&gs->perViewUB, sizeof(PerViewUniforms));
	uniformBuffer_init(&gs->perFrameUB, sizeof(PerFrameUniforms));
	
	
	printf("diffuse2: %d\n",gs->diffuseTexBuffer);
	// set up the Geometry Buffer

	initRenderLoop(gs);
	
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
	getPrintGLEnum(GL_MIN_PROGRAM_TEXEL_OFFSET, "meh");
	getPrintGLEnum(GL_MAX_PROGRAM_TEXEL_OFFSET, "meh");
	getPrintGLEnum(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, "meh");
	getPrintGLEnum(GL_MAX_UNIFORM_BLOCK_SIZE, "meh");
	
	
	
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
	
	
	initStaticMeshes();
	initRoads();
	initEmitters();
	
	// initialize all those magic globals
// 	initMap(&gs->map);
	scene_init(&gs->scene);
	
	
	gui_Init();
	
	gt = guiTextNew("gui!", &(Vector){1.0,1.0,0.0}, 6.0f, "Arial");
	gt_sel = guiTextNew("gui!", &(Vector){1.0,1.6,0.0}, 6.0f, "Arial");
	gt_emit = guiTextNew("gui!", &(Vector){1.0,2.2,0.0}, 6.0f, "Arial");
	gtRenderMode = guiTextNew("", &(Vector){8.0,1.0,0.0}, 6.0f, "Arial");
	gtSelectionDisabled = guiTextNew("", &(Vector){8.0,2.0,0.0}, 6.0f, "Arial");
	
	
	//initUI(gs);
	initMarker();
	 

	
	gs->world = calloc(1, sizeof(*gs->world));
	World_init(gs->world);
	gs->world->gs = gs;
	
/*	
	dust = makeEmitter();
	EmitterInstance dust_instance = {
		.pos = {250.0,250.0,250.0},
		.scale = 10,
		.start_time = 0,
		.lifespan = 1<<15
	};
	
	emitterAddInstance(dust, &dust_instance);
	emitter_update_vbo(dust);*/
	
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
	
	// update per-frame timer values
	
	PerFrameUniforms* pfu = uniformBuffer_begin(&gs->perFrameUB);
	
	double seconds = (float)(long)gs->frameTime;
	
	pfu->wholeSeconds = seconds;
	pfu->fracSeconds = gs->frameTime - seconds;
	
	uniformBuffer_bindRange(&gs->perFrameUB);
	
	static double sdtime, sseltime, semittime;
	
	if(lastPoint == 0.0f) lastPoint = gs->frameTime;
	if(1 /*frameCounter == 0*/) {
		float fps = 60.0f / (gs->frameTime - lastPoint);
		
		uint64_t qtime;
		
		if(!query_queue_try_result(&gs->queries.draw, &qtime)) {
			sdtime = ((double)qtime) / 1000000.0;
		}
		snprintf(frameCounterBuf, 128, "dtime:  %.2fms", sdtime);
		guiTextSetValue(gt, frameCounterBuf);


		if(!query_queue_try_result(&gs->queries.selection, &qtime)) {
			sseltime = ((double)qtime) / 1000000.0;
		}
		snprintf(frameCounterBuf, 128, "seltime:  %.2fms", sseltime);
		guiTextSetValue(gt_sel, frameCounterBuf);
		
		
		if(!query_queue_try_result(&gs->queries.emitters, &qtime)) {
			semittime = ((double)qtime) / 1000000.0;
		}
		snprintf(frameCounterBuf, 128, "emittime:  %.2fms", semittime);
		guiTextSetValue(gt_emit, frameCounterBuf);
		
		
		lastPoint = now;
	}
	
	
	// check the pbos
	GLenum val;
	if(gs->selectionFence) {
		val = glClientWaitSync(gs->selectionFence, 0, 0);
		
		if(val == GL_CONDITION_SATISFIED || val == GL_ALREADY_SIGNALED) {
			printf("signaled %d\n", gs->frameCount - gs->selectionFrame);
			glDeleteSync(gs->selectionFence);
			gs->selectionFence = 0;
			
			glBindBuffer(GL_PIXEL_PACK_BUFFER, gs->selectionPBOs[gs->activePBO]);
			void* p = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, 600*600*4, GL_MAP_READ_BIT);
			glexit("buffer map");
			if(p == NULL) {
				exit(1);
			}
			
			int sz = gs->screen.wh.x * gs->screen.wh.y * 4; 
			memcpy(gs->selectionData, p, sz);
			
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			
			
			gs->readPBO = gs->activePBO;
			gs->activePBO = (gs->activePBO + 1) % 2;
		}
	}
}




void postFrame(GameState* gs) {
	
	double now;
	
	now = getCurrentTime();
	
	gs->perfTimes.draw = now - gs->frameTime;
	
	uniformBuffer_finish(&gs->perFrameUB);
}


void handleInput(GameState* gs, InputState* is) {
	
	double te = gs->frameSpan;
	
	float moveSpeed = gs->settings.keyScroll * te; // should load from config
	float rotateSpeed = gs->settings.keyRotate * te; // 20.8 degrees
	float keyZoom = gs->settings.keyZoom * te;
	float mouseZoom = gs->settings.mouseZoom * te;

	if(is->keyState[54] & IS_KEYDOWN) {
		exit(0);
	}
	
	if(is->clickButton == 1) {
		World_spawnAt_StaticMesh(gs->world, 0, &gs->cursorPos);
		
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
	static char* modeStrings[] = {
		"",
		"diffuse",
		"normal",
		"depth",
		"lighting"
	};
	if(is->keyState[119] & IS_KEYDOWN) {
		if(gs->frameTime > lastChange + 1) {
			gs->debugMode = (gs->debugMode + 1) % 5;
			lastChange = gs->frameTime;

			guiTextSetValue(gtRenderMode, modeStrings[gs->debugMode]);

		}
	}
	
	if(is->keyState[33] & IS_KEYDOWN) {
		gs->selectionPassDisabled = !gs->selectionPassDisabled;

		if(gs->selectionPassDisabled) {
			guiTextSetValue(gtSelectionDisabled, "Selection Disabled");
		}
		else {
			guiTextSetValue(gtSelectionDisabled, "");
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
	//renderUIPicking(xs, gs);
	
	
	
	//updateView(xs, gs, is);
	

	// draw terrain
// 	drawTerrainBlockDepth(&gs->map, msGetTop(&gs->model), msGetTop(&gs->view), msGetTop(&gs->proj));
	drawTerrainDepth(&gs->scene.map, &gs->perViewUB, &gs->screen.wh);
	
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
	
	// TODO: only update if somethign changes
	PerViewUniforms* pvu = uniformBuffer_begin(&gs->perViewUB);
	
	memcpy(&pvu->view , msGetTop(&gs->view), sizeof(Matrix));
	memcpy(&pvu->proj , msGetTop(&gs->proj), sizeof(Matrix));
	
	uniformBuffer_bindRange(&gs->perViewUB);
}





void checkCursor(GameState* gs, InputState* is) {
	
	union {
		unsigned char rgb[4];
		uint32_t in;
	} u;
	glexit("pre selection buff");
	
	if(!gs->selectionData) return;
	
	int w = (int)gs->screen.wh.x;
	int h = (int)gs->screen.wh.y;
	
	int x = (int)is->cursorPosPixels.x;
	int y = (int)is->cursorPosPixels.y;

	//printf("cursor %f, %f\n",is->cursorPosPixels.x, is->cursorPosPixels.y);
	int i = x + (y * w);
	//printf("off %d %f \n", i, gs->screen.wh.x);
	uint32_t j = gs->selectionData[i];
	u.in = j;
	//printf("j %d\n", j); 
	
	gs->cursorTilePos.x = u.rgb[0];
	gs->cursorTilePos.y = u.rgb[1];
	gs->cursorTilePos.z = u.rgb[2];
	
	struct sGL_RG8* off = &gs->scene.map.offsetData[(int)gs->cursorTilePos.z]; 
	
	gs->cursorPos.x = (off->x * 256.0) + gs->cursorTilePos.x;
	gs->cursorPos.y = (off->y * 256.0) + gs->cursorTilePos.y;
	
	
	//printf("pos: x: %d, y:%d \n", (int)gs->cursorPos.x, (int)gs->cursorPos.y);
	/*
 	printf("mx: %d, my: %d, x: %d, y: %d, z: %d\n", 
		   (int)is->cursorPosPixels.x, 
		   (int)is->cursorPosPixels.y,  
		   rgb[0], rgb[1], rgb[2]);
	*/
	
	if(is->clickButton == 3 && u.rgb[2] == 1) {
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
	
	checkCursor(gs, is);
	
	
	drawFrame(xs, gs, is);
	
	
	gs->screen.resized = 0;

	postFrame(gs);
}
