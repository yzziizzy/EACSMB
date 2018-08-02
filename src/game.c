
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <time.h>



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
#include "dynamicMesh.h"
#include "waterPlane.h"
#include "road.h"
#include "map.h"
#include "game.h"
#include "emitter.h"
#include "scene.h"
#include "world.h"
#include "gui.h"
#include "ui/simpleWindow.h"
#include "ui/image.h"
#include "builder/render.h"
#include "texgen.h"

#include "sexp.h"


GLuint proj_ul, view_ul, model_ul;

Matrix mProj, mView, mModel;

float zoom;


// temp shit
GUIText* gt;
GUIText* gt_terrain;
GUIText* gt_solids;
GUIText* gt_selection;
GUIText* gt_decals;
GUIText* gt_emitters;
GUIText* gt_lighting;
GUIText* gt_shading;
GUIText* gt_gui;
GUIText* gtRenderMode;
GUIText* gtSelectionDisabled;
GUISimpleWindow* gswTest;
GUIImage* giTest;

GUIEdit* geditTest;

GUIRenderTarget* grtTest;
GUIBuilderControl* gbcTest;
GUITexBuilderControl* texbuilder;
Texture* cnoise;
Emitter* dust;



RenderPipeline* rpipe;

ShaderProgram* erodeProg;



// in renderLoop.c, a temporary factoring before a proper renderer is designed
void drawFrame(XStuff* xs, GameState* gs, InputState* is);
void setupFBOs(GameState* gs, int resized);


// MapBlock* map;
// TerrainBlock* terrain;


static void main_drag_handler(InputEvent* ev, GameState* gs);
static void main_key_handler(InputEvent* ev, GameState* gs);
static void main_perframe_handler(InputState* is, float frameSpan, GameState* gs);
static void main_click_handler(InputEvent* ev, GameState* gs);



void initGame(XStuff* xs, GameState* gs) {
	int ww, wh;
	
	srand((unsigned int)time(NULL));
	
	json_gl_init_lookup();
	
	glerr("left over error on game init");
	
	CES_init(&gs->ces);
	
	CES_addComponentManager(&gs->ces, ComponentManager_alloc("position", sizeof(Vector), 1024*8));
	CES_addComponentManager(&gs->ces, ComponentManager_alloc("meshIndex", sizeof(uint16_t), 1024*8));
	CES_addComponentManager(&gs->ces, ComponentManager_alloc("rotation", sizeof(C_Rotation), 1024*8));
	
	// about axis of rotation
	CES_addComponentManager(&gs->ces, ComponentManager_alloc("angularVelocity", sizeof(float), 1024*8));
	
	CES_addComponentManager(&gs->ces, ComponentManager_alloc("pathFollow", sizeof(C_PathFollow), 1024*8));
	
	erodeProg = loadCombinedProgram("mg_erode");
	
	gs->hasMoved = 1;
	gs->lastSelectionFrame = 0;
	gs->frameCount = 0;
	
	gs->activeTool = 0;
	
	gs->debugMode = 0;
	gs->sunSpeed = 0;
	gs->sunTheta = 4;
	
	gs->nearClipPlane = 3;
	gs->farClipPlane = 1700;

	
	ww = xs->winAttr.width;
	wh = xs->winAttr.height;
	
	gs->screen.wh.x = (float)ww;
	gs->screen.wh.y = (float)wh;
	
	gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
	gs->screen.resized = 0;
	
	gs->defaultInputHandlers = calloc(1, sizeof(*gs->defaultInputHandlers));
	gs->defaultInputHandlers->dragStop = main_drag_handler;
	gs->defaultInputHandlers->keyUp = main_key_handler;
	gs->defaultInputHandlers->perFrame = main_perframe_handler;
	gs->defaultInputHandlers->click = main_click_handler;
	InputFocusStack_PushTarget(&gs->ifs, gs, defaultInputHandlers);
	
	//printf("w: %d, h: %d\n", ww, wh);
	
	
	initUniformBuffers();
	
	uniformBuffer_init(&gs->perViewUB, sizeof(PerViewUniforms));
	uniformBuffer_init(&gs->perFrameUB, sizeof(PerFrameUniforms));
	
	
	//printf("diffuse2: %d\n",gs->diffuseTexBuffer);
	// set up the Geometry Buffer

	initRenderLoop(gs);
	initRenderPipeline();
	
	// check some prerequisites
// 	GLint MaxDrawBuffers = 0;
// 	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MaxDrawBuffers);
// 	printf("MAX_DRAW_BUFFERS: %d\n", MaxDrawBuffers);
//  	if(MaxDrawBuffers < 4) {
// 		fprintf(stderr, "FATAL: GL_MAX_DRAW_BUFFERS is too low: %d. Minimum required value is 4.\n", MaxDrawBuffers);
// 		exit(3);
// 	};

	/*
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
	getPrintGLEnum(GL_MAX_TEXTURE_SIZE, "meh");
	
	*/
	
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
	
	
	
	
	initTextures();
	
	initStaticMeshes(); // static meshes will probably be phased out due to the culling efficiency of dynamic meshes
	initDynamicMeshes();
	initLighting();
	//initRoads();
	initWaterPlane();
	initEmitters();
	initDecals();
	initCustomDecals();
	
	
	// initialize all those magic globals
// 	initMap(&gs->map);
	Scene_init(&gs->scene);
	
	
	gui_Init();
	
	gt = guiTextNew("gui!", (Vector2){0.010,0.01}, 4.0f, "Arial");
	gt_terrain = guiTextNew("gui!", (Vector2){0.010,0.04}, 3.0f, "Arial");
	gt_solids = guiTextNew("gui!", (Vector2){0.010,0.06}, 3.0f, "Arial");
	gt_selection = guiTextNew("gui!", (Vector2){0.010,0.28}, 3.0f, "Arial");
	gt_decals = guiTextNew("gui!", (Vector2){0.010,0.08}, 3.0f, "Arial");
	gt_emitters = guiTextNew("gui!", (Vector2){0.010,0.10}, 3.0f, "Arial");
	gt_lighting = guiTextNew("gui!", (Vector2){0.010,0.12}, 3.0f, "Arial");
	gt_shading = guiTextNew("gui!", (Vector2){0.010,0.14}, 3.0f, "Arial");
	gt_gui = guiTextNew("gui!", (Vector2){0.010,0.16}, 3.0f, "Arial");
	gtRenderMode = guiTextNew("", (Vector2){0.1,0.9}, 6.0f, "Arial");
	gtSelectionDisabled = guiTextNew("", (Vector2){0.5,0.1}, 6.0f, "Arial");
	
	

// 	gwTest = guiWindowNew((Vector2){.2, .2}, (Vector2){.7, .7}, 0);
// 	gwTest->header.onClick = testClick;
	
	gswTest = guiSimpleWindowNew((Vector2){.2, .2}, (Vector2){.7, .7}, 0);
	//gswTest->header.onClick = testClick;
	
	giTest = guiImageNew((Vector2){.1,.2}, (Vector2){.8,.8}, 0, 0);
	
	
	//guiRegisterObject(gswTest, NULL);
	//guiRegisterObject(gt, NULL);
	guiRegisterObject(gt_terrain, NULL);
	guiRegisterObject(gt_solids, NULL);
	//guiRegisterObject(gt_selection, NULL);
	guiRegisterObject(gt_decals, NULL);
	guiRegisterObject(gt_emitters, NULL);
	guiRegisterObject(gt_lighting, NULL);
	guiRegisterObject(gt_shading, NULL);
	guiRegisterObject(gt_gui, NULL);
	
	guiRegisterObject(gtRenderMode, NULL);
	guiRegisterObject(gtSelectionDisabled, NULL);
	

	//geditTest = GUIEditNew("edit", (Vector2){.5, .5}, (Vector2){.4, .05});
	//guiRegisterObject(geditTest, NULL);
	//InputFocusStack_PushTarget(&gs->ifs, geditTest, inputHandlers);

	
	gs->world = calloc(1, sizeof(*gs->world));
	gs->world->gs = gs;
	World_init(gs->world);
	
	
	
// 	init_Builder();

}



void preFrame(GameState* gs) {
	
// 	// HACK
// 	if(bpipe->backingTextures) {
// 		//printf("\n\n-----------\nbacking textures\n\n\n\n");
// 			
// 	//printf("output 2 tex: %d \n", bpipe->backingTextures[4]);
// 		giTest->customTexID = bpipe->backingTextures[4];
// 	}
// 	
// 	
//	if(rpipe)
//		grtTest->texID = RenderPipeline_getOutputTexture(rpipe);
	
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

#define query_update_gui(qname)		\
		if(!query_queue_try_result(&gs->queries.qname, &qtime)) {\
			sdtime = ((double)qtime) / 1000000.0;\
		}\
		snprintf(frameCounterBuf, 128, #qname ":  %.2fms", sdtime);\
		guiTextSetValue(gt_##qname, frameCounterBuf);


		query_update_gui(terrain);
		query_update_gui(solids);
		query_update_gui(selection);
		query_update_gui(decals);
		query_update_gui(emitters);
		query_update_gui(shading);
		query_update_gui(lighting);
		query_update_gui(gui);
		
		//if(!query_queue_try_result(&gs->queries.draw, &qtime)) {
			//sdtime = ((double)qtime) / 1000000.0;
		//}
		//snprintf(frameCounterBuf, 128, "dtime:  %.2fms", sdtime);
		//guiTextSetValue(gt, frameCounterBuf);


		//if(!query_queue_try_result(&gs->queries.selection, &qtime)) {
			//sseltime = ((double)qtime) / 1000000.0;
		//}
		//snprintf(frameCounterBuf, 128, "seltime:  %.2fms", sseltime);
		//guiTextSetValue(gt_sel, frameCounterBuf);
		
		
		//if(!query_queue_try_result(&gs->queries.emitters, &qtime)) {
			//semittime = ((double)qtime) / 1000000.0;
		//}
		//snprintf(frameCounterBuf, 128, "emittime:  %.2fms", semittime);
		//guiTextSetValue(gt_emit, frameCounterBuf);
		
		lastPoint = now;
	}
	
	
	// check the pbos
	GLenum val;
	if(gs->selectionFence) {
		val = glClientWaitSync(gs->selectionFence, 0, 0);
		
		if(val == GL_CONDITION_SATISFIED || val == GL_ALREADY_SIGNALED) {
			//printf("signaled %d\n", gs->frameCount - gs->selectionFrame);
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



static void main_perframe_handler(InputState* is, float frameSpan, GameState* gs) {
	double te = gs->frameSpan;
	
	float moveSpeed = gs->settings.keyScroll * te; // should load from config
	float rotateSpeed = gs->settings.keyRotate * te; // 20.8 degrees
	float keyZoom = gs->settings.keyZoom * te;
	float mouseZoom = gs->settings.mouseZoom * te;

	
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
		gs->nearClipPlane -= fmax(50 * te, 0.1);
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
	
}



static void main_drag_handler(InputEvent* ev, GameState* gs) {
	
	printf("dragged from %d,%d to %d,%d \n", ev->intDragStart.x, ev->intDragStart.y,
		ev->intPos.x, ev->intPos.y
	);
	printf("dragged from %f,%f to %f,%f \n", ev->normDragStart.x, ev->normDragStart.y,
		ev->normPos.x, ev->normPos.y
	);
	
	Vector2i tile;
	getTileFromScreenCoords(gs, ev->normPos, &tile);
	Vector2 to = {tile.x, tile.y};
	getTileFromScreenCoords(gs, ev->normDragStart, &tile);
	Vector2 from = {tile.x, tile.y};
	
	World_spawnAt_Road(gs->world, &from, &to);
	
}

static void main_key_handler(InputEvent* ev, GameState* gs) {
	
	if(ev->character == 'c') {
		exit(0);
	}
	
	if(ev->keysym == XK_Delete) {
		static int lastChange = 0;
		static char* modeStrings[] = {
			"",
			"diffuse",
			"normal",
			"depth",
			"selection",
			"lighting"
		};
		
		gs->debugMode = (gs->debugMode + 1) % 6;
		lastChange = gs->frameTime;
		
		guiTextSetValue(gtRenderMode, modeStrings[gs->debugMode]);
	}
	
	if(ev->keysym == XK_Insert) {
		gs->selectionPassDisabled = !gs->selectionPassDisabled;
		guiTextSetValue(gtSelectionDisabled, gs->selectionPassDisabled ? "Selection Disabled" : "");
	}
	
	if(ev->character == 'b') {
		// builder control
		gbcTest = guiBuilderControlNew((Vector2){.1,.2}, (Vector2){.8,.8}, 0);
		guiRegisterObject(gbcTest, NULL);
		guiResize(&gbcTest->header, (Vector2){.79, .79});
		guiRenderTarget_SetScreenRes(gbcTest->rt,  (Vector2i){gs->screen.wh.x, gs->screen.wh.y});
		
		InputFocusStack_PushTarget(&gs->ifs, gbcTest, inputHandlers);
	}
	
	// texture builder
	if(ev->character == 't') {
		printf("t\n");
		texbuilder = guiTexBuilderControlNew((Vector2){.1,.2}, (Vector2){.8,.8}, 0);
		guiRegisterObject(texbuilder, NULL);
		guiResize(&texbuilder->header, (Vector2){.79, .79});
		
		InputFocusStack_PushTarget(&gs->ifs, texbuilder, inputHandlers);
	}
	
	if(ev->character == 'k') {
		
		
	}
}  


static void main_click_handler(InputEvent* ev, GameState* gs) {

	if(ev->button == 1) {
		
		// BUG: used inverse cursor pos. changed to compile temporarily
		GUIObject* hit = guiHitTest(gswTest, ev->normPos);
		printf("\n\n----> %f, %f \n", ev->normPos.x, ev->normPos.y);
		if(0 && hit) {
			printf("@@clicked in window \n");
			
			GUIEvent e;
			e.originalTarget = hit;
			e.currentTarget = hit;
			e.eventPos = ev->normPos; // BUG: smae here
			
			guiTriggerClick(&e);
		}
		else {
			Vector2i tile;
			Vector2 invPos = {ev->normPos.x, 1 - ev->normPos.y};
			getTileFromScreenCoords(gs, ev->normPos, &tile);
			printf("x: %d, y: %d\n", tile.x, tile.y);
			Vector2 tilef = {tile.x, tile.y};
			//BUG: convert this to tile coords
			World_spawnAt_Item(gs->world, "gazebbq", &tilef);
		}
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
}

void handleInput(GameState* gs, InputState* is) {
	
	printf("\n------ handInput is deprecated ------\n\n");
	
	
	static int dragstart = -1;
	if(is->buttonDown == 1) {
		gs->mouseDownPos.x = gs->cursorPos.x;
		gs->mouseDownPos.y = gs->cursorPos.y;
		dragstart = getCurrentTime();
		//printf("start dragging at (%d,%d)\n", (int)gs->cursorPos.x, (int)gs->cursorPos.y);
	}
	if(is->buttonUp == 1) {
		double dragtime = timeSince(dragstart);
		if(dragtime < 0.7) {
			//printf("ignoring drag, too short: %.8f\n", dragtime);
		}
		else {
		//vCopy(&gs->cursorPos, &gs->mouseDownPos);
			//printf("stopped dragging at (%d,%d)\n", (int)gs->cursorPos.x, (int)gs->cursorPos.y);
			
			World_spawnAt_Road(gs->world, &gs->mouseDownPos, &gs->cursorPos);
		}
		
		
		
	}
	
	if(is->clickButton == 2) {
		gs->activeTool = (gs->activeTool + 1) % 3;
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
	drawTerrainDepth(&gs->world->map, &gs->perViewUB, &gs->screen.wh);
	
	//msPop(&gs->view);
	//msPop(&gs->proj);
	
}


void updateView(XStuff* xs, GameState* gs, InputState* is) {
		
	gs->sunTheta = fmod(gs->sunTheta + gs->sunSpeed * gs->frameSpan, F_2PI);
	gs->sunNormal.x = cos(gs->sunTheta);
	gs->sunNormal.y = 0.0;
	gs->sunNormal.z = sin(gs->sunTheta);
	
	//printf("sun theta %f\n", gs->sunTheta);
	
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
	
	Vector zero = {0,0,0};
	vMatrixMul(&zero, &invv, &gs->eyePos);
	
	// TODO: only update if somethign changes
	PerViewUniforms* pvu = uniformBuffer_begin(&gs->perViewUB);
	
	memcpy(&pvu->view , msGetTop(&gs->view), sizeof(Matrix));
	memcpy(&pvu->proj , msGetTop(&gs->proj), sizeof(Matrix));	
	
	uniformBuffer_bindRange(&gs->perViewUB);
}



void getTileFromScreenCoords(GameState* gs, Vector2 scoord, Vector2i* tile) {
	
	union {
		unsigned char rgb[4];
		uint32_t in;
	} u;
	
	if(!gs->selectionData) return;
	
	int w = (int)gs->screen.wh.x;
	int h = (int)gs->screen.wh.y;
	
	int i = (scoord.x * w) + ((scoord.y * h) * w);
	
	uint32_t j = gs->selectionData[i];
	u.in = j;
	
	gs->cursorTilePos.x = u.rgb[0];
	gs->cursorTilePos.y = u.rgb[1];
	gs->cursorTilePos.z = u.rgb[2];
	
	struct sGL_RG8* off = &gs->world->map.offsetData[(int)gs->cursorTilePos.z]; 
	//printf("*tile offset: %u - %d - %d,%d,%d - %f,%f\n", j, u.rgb[2], (int)gs->cursorTilePos.z, off->x, off->y,
	//	scoord.x, scoord.y
	//);
	
	tile->x = (off->x * 256.0) + gs->cursorTilePos.x;
	tile->y = (off->y * 256.0) + gs->cursorTilePos.y;

}


// deprecated, use above
void checkCursor(GameState* gs, InputState* is) {
	
	union {
		unsigned char rgb[4];
		uint32_t in;
	} u;
	glexit("pre selection buff");
	
	if(!gs->selectionData) return;
	
	int w = (int)gs->screen.wh.x;
	int h = (int)gs->screen.wh.y;
	
	int x = (int)is->lastCursorPosPixels.x;
	int y = (int)is->lastCursorPosPixels.y;

	//printf("cursor %f, %f\n",is->cursorPosPixels.x, is->cursorPosPixels.y);
	int i = x + (y * w);
	//printf("off %d %f \n", i, gs->screen.wh.x);
	uint32_t j = gs->selectionData[i];
	u.in = j;
	//printf("j %d\n", j); 
	
	gs->cursorTilePos.x = u.rgb[0];
	gs->cursorTilePos.y = u.rgb[1];
	gs->cursorTilePos.z = u.rgb[2];
	
	//struct sGL_RG8* off = &gs->world->map.offsetData[(int)gs->cursorTilePos.z]; 
	
	// NOTE: modified to not segfault during map update
	gs->cursorPos.x =  gs->cursorTilePos.x;
	gs->cursorPos.y =  gs->cursorTilePos.y;
	
	
	//printf("tile offset: %u - %d - %d,%d,%d - %d,%d\n", j, u.rgb[2], (int)gs->cursorPos.x, (int)gs->cursorPos.y,
		//(int)is->lastCursorPosPixels.x, (int)is->lastCursorPosPixels.y
	//);
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
		
		//printf("screen 0 resized\n");
		
		viewWH.x = xs->winAttr.width;
		viewWH.y = xs->winAttr.height;
		
		gs->screen.wh.x = (float)xs->winAttr.width;
		gs->screen.wh.y = (float)xs->winAttr.height;
		
		gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
		
		gs->screen.resized = 1;
		
		setupFBOs(gs, 1);
		
		if(gbcTest)
			guiRenderTarget_SetScreenRes(gbcTest->rt, (Vector2i){gs->screen.wh.x, gs->screen.wh.y});
		//printf("diffuse2: %d\n",gs->diffuseTexBuffer);
	}
}


// temp hack
void runSystems(GameState* gs, InputState* is) {
	
	
	// angular rotation
	ComponentManager* avComp = CES_getCompManager(&gs->ces, "angularVelocity");
	ComponentManager* rotComp = CES_getCompManager(&gs->ces, "rotation");
	
	int avindex = -1;
	int rindex = -1;
	uint32_t eid;
	float* av;
	while(av = ComponentManager_next(avComp, &avindex, &eid)) {
		//printf("eid %d %d %d\n", eid, cindex, pindex);
		C_Rotation* rot;
		if(!(rot = ComponentManager_nextEnt(rotComp, &rindex, eid))) {
			 continue;
		}
		
		rot->theta = fmod(rot->theta + (gs->frameSpan * *av), F_2PI);
	}
		
	// --------------------------------
	
	ComponentManager* posComp = CES_getCompManager(&gs->ces, "position");
	ComponentManager* pathComp = CES_getCompManager(&gs->ces, "pathFollow");
	
	int findex = -1;
	int pindex = -1;
	eid = 0;
	C_PathFollow* pf;
	while(pf = ComponentManager_next(pathComp, &findex, &eid)) {
		//printf("eid %d %d %d\n", eid, cindex, pindex);
		Vector* pos;
		if(!(pos = ComponentManager_nextEnt(posComp, &pindex, eid))) {
			 continue;
		}
		
		pf->distTravelled += pf->speed * gs->frameSpan;
		Vector2 p2 = Path_GetPos(pf->path, pf->distTravelled);
		
		pos->x = p2.x;
		pos->y = p2.y;
		pos->z = getTerrainHeightf(&gs->world->map, &p2); 
	}
	
}





#define PF_START(x) gs->perfTimes.x = getCurrentTime()
#define PF_STOP(x) gs->perfTimes.x = timeSince(gs->perfTimes.x)

void gameLoop(XStuff* xs, GameState* gs, InputState* is) {
	gs->frameCount++;
	
	MapGen_erode(&gs->world->map, erodeProg);
	
	checkResize(xs,gs);
	
		PF_START(preframe);
	preFrame(gs);
		PF_STOP(preframe);
	
	//handleInput(gs, is);
	InputFocusStack_DispatchPerFrame(&gs->ifs, is, gs->frameSpan);
	
	//setUpView(gs);
	updateView(xs, gs, is);
	
	checkCursor(gs, is);
	
	
	runSystems(gs, is);
	
	
	drawFrame(xs, gs, is);
	
	
	gs->screen.resized = 0;

	postFrame(gs);
}
