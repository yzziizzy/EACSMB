
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
#include "c_json/json.h"
#include "json_gl.h"

#include "mempool.h"

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
GUIText* gt_effects;
GUIText* gt_lighting;
GUIText* gt_sunShadow;
GUIText* gt_shading;
GUIText* gt_gui;

GUIWindow* gw_test;
GUIWindow* gw_test2;

GUIImage* gt_img;

GUIText* gtRenderMode;
GUIText* gtSelectionDisabled;
GUISimpleWindow* gswTest;
GUIImage* giTest;

GUIColumnLayout* gclTest;
GUIGridLayout* gglTest;

GUIEdit* geditTest;

// GUIImageButton* gibTest;

GUIRenderTarget* grtTest;
GUIBuilderControl* gbcTest;
GUITexBuilderControl* texbuilder;
Texture* cnoise;
// Emitter* dust;

RenderPipeline* rpipe;

ShaderProgram* waterProg;
ShaderProgram* erodeProg;
ShaderProgram* soilProg;



// in renderLoop.c, a temporary factoring before a proper renderer is designed
void drawFrame(XStuff* xs, GameState* gs, InputState* is);
void setupFBOs(GameState* gs, int resized);


// MapBlock* map;
// TerrainBlock* terrain;


static void main_drag_handler(InputEvent* ev, GameState* gs);
static void main_key_handler(InputEvent* ev, GameState* gs);
static void main_perframe_handler(InputState* is, float frameSpan, GameState* gs);
static void main_click_handler(InputEvent* ev, GameState* gs);
static void main_move_handler(InputEvent* ev, GameState* gs);


// nothing in here can use opengl at all.
void initGame(XStuff* xs, GameState* gs) {
	
	srand((unsigned int)time(NULL));
	
	json_gl_init_lookup();
	
	
// 	TextureAtlas* ta = TextureAtlas_alloc();
// 	ta->width = 256;
// 	TextureAtlas_addFolder(ta, "pre", "assets/ui/icons", 0);
// 	TextureAtlas_finalize(ta);
// 	
	
	gs->gui = GUIManager_alloc(&gs->globalSettings);
	
	
	// sound
#ifndef DISABLE_SOUND
	gs->sound = SoundManager_alloc();
	SoundManager_readConfigFile(gs->sound, "assets/config/sound.json");
	SoundManager_start(gs->sound);
	
// 	SoundClip* sc = SoundClip_fromWAV("./assets/sounds/ohno.wav");
	SoundClip* sc = SoundClip_fromVorbis("./assets/sounds/ooo.ogg");
	SoundManager_addClip(gs->sound, sc, "ohno");

	
#endif


	// CES system

	struct { 
		char* name;
		size_t size;
	} defaultComponents[] = {
		{"meshIndex", sizeof(uint16_t)},
		{"position", sizeof(Vector)},
		{"relativeInfo", sizeof(C_RelativeInfo)},
		{"rotation", sizeof(C_Rotation)},
		{"mapHeightUpdate", sizeof(uint8_t)},
		{"angularVelocity", sizeof(float)},
		{"customDecal", sizeof(CustomDecalInstance)},
		{"agent", sizeof(C_Agent)},
		{"pathFollow", sizeof(C_PathFollow)},
		{"roadWander", sizeof(C_RoadWander)},
		{0, 0},
	};
	
	
	CES_init(&gs->ces);
	
	for(int i = 0; defaultComponents[i].name != NULL; i++) {
		CES_addComponentManager(&gs->ces, 
			ComponentManager_alloc(defaultComponents[i].name, defaultComponents[i].size, 1024*8, 1)
		);
	}
	
	json_file_t* j_ces_conf = json_load_path("assets/config/CES.json");
	ComponentManager_loadConfig(&gs->ces, j_ces_conf->root);
	
	json_free(j_ces_conf->root);
	free(j_ces_conf);
	
	//^^^^^^^^^^ CES system ^^^^^^^^^^^^^
	/*
	
	float fff = 33.33;
	for(int i = 0; i < 6000; i++) { 
		
		CES_addComponentName(&gs->ces, "angularVelocity", i, &fff);
	}
	
	for(int i = 1000; i < 1024; i+=1) { 
		CES_delComponentName(&gs->ces, "angularVelocity", i);
	}
	
	printf("debug exit at %s:%d\n", __FILE__, __LINE__);
	exit(1);
	
	*/
	// ces delete test ^^^^^^^^^^
	
	
	
	// input handlers
	gs->defaultInputHandlers = calloc(1, sizeof(*gs->defaultInputHandlers));
	gs->defaultInputHandlers->dragStop = (void*)main_drag_handler;
	gs->defaultInputHandlers->keyUp = (void*)main_key_handler;
	gs->defaultInputHandlers->perFrame = (void*)main_perframe_handler;
	gs->defaultInputHandlers->click = (void*)main_click_handler;
	gs->defaultInputHandlers->mouseMove = (void*)main_move_handler;
	InputFocusStack_PushTarget(&gs->ifs, gs, defaultInputHandlers);
	
	
	// general properties
	const float rotateFactor = 0.7260f;
	const float scrollFactor = 300.0f;
	const float zoomFactor = 600.0f;
	
	gs->settings.keyRotate = rotateFactor * fclampNorm(gs->globalSettings.keyRotateSensitivity);
	gs->settings.keyScroll = scrollFactor * fclampNorm(gs->globalSettings.keyScrollSensitivity);
	gs->settings.keyZoom = zoomFactor * fclampNorm(gs->globalSettings.keyZoomSensitivity);
	
	gs->settings.mouseRotate = rotateFactor * fclampNorm(gs->globalSettings.mouseRotateSensitivity);
	gs->settings.mouseScroll = scrollFactor * fclampNorm(gs->globalSettings.mouseScrollSensitivity);
	gs->settings.mouseZoom = 4 * zoomFactor * fclampNorm(gs->globalSettings.mouseZoomSensitivity);
	
	gs->hasMoved = 1;
	gs->lastSelectionFrame = 0;
	gs->frameCount = 0;
	
	gs->activeTool = 0;
	
	gs->debugMode = 0;
	gs->sunSpeed = 0;
	gs->sunTheta = 2.2;
	
	gs->nearClipPlane = .5;
	gs->farClipPlane = 1700;

	int ww, wh;
	ww = xs->winAttr.width;
	wh = xs->winAttr.height;
	
	gs->screen.wh.x = (float)ww;
	gs->screen.wh.y = (float)wh;
	gs->gui->screenSize = (Vector2i){ww, wh};
	
	gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
	gs->screen.resized = 0;
	
	gs->zoom = -600.0;
	gs->direction = 0.0f;
	gs->lookCenter.x = 128;
	gs->lookCenter.y = 128;
	
	
	// set up matrix stacks
	MatrixStack* view, *proj;
	
	view = &gs->view;
	proj = &gs->proj;
	
	msAlloc(2, view);
	msAlloc(2, proj);

	msIdent(view);
	msIdent(proj);
	
	
	#include "../mods/GameState_init.generated_thunk.c" 
	
	
	gs->world = calloc(1, sizeof(*gs->world));
	gs->world->gs = gs;
	World_init(gs->world);

}

void initGameGL(XStuff* xs, GameState* gs) {
	
	
	glerr("left over error on game init");
	
	
	GUIManager_initGL(gs->gui, &gs->globalSettings);
	gs->guiPass = GUIManager_CreateRenderPass(gs->gui);
	
	/*
	
	height auto-update flag
	
	money, debt
	food consumption
	gratification deferment
	
	*/
	
	waterProg = loadCombinedProgram("mg_water");
	erodeProg = loadCombinedProgram("mg_erode");
	soilProg = loadCombinedProgram("mg_soil");
	

	
	initUniformBuffers();
	
	uniformBuffer_init(&gs->perViewUB, sizeof(PerViewUniforms));
	uniformBuffer_init(&gs->perFrameUB, sizeof(PerFrameUniforms));

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
	
	
	
	initTextures();
	
	initStaticMeshes(); // static meshes will probably be phased out due to the culling efficiency of dynamic meshes
	initDynamicMeshes();
	initLighting();
	//initRoads();
	initWaterPlane();
// 	initEmitters();
	initDecals();
	initCustomDecals();
	
	
	// initialize all those magic globals
// 	initMap(&gs->map);
	Scene_init(&gs->scene);
	
	
	gt = GUIText_new(gs->gui, "", "Arial", 3.0f);
	
	gtRenderMode = GUIText_new(gs->gui, "", "Arial", 6.0f);
	gtSelectionDisabled = GUIText_new(gs->gui, "", "Arial", 6.0f);
	

	
	char* iconNames[] = {
		"pre/audio",
		"pre/plane",
		"pre/bag",
		"pre/book",
		"pre/calculator",
		
		"pre/camera",
		"pre/car",
		"pre/check",
		"pre/clock",
		"pre/cloud",
		
		"pre/crop",
		"pre/cup",
		"pre/cutlery",
		"pre/denied",
		"pre/down_arrow",
		
		"pre/entrance",
		"pre/envelope",
		"pre/file",
		"pre/gear",
		"pre/home",
	};
	
	
	
	
	gglTest = GUIGridLayout_new(gs->gui, (Vector2){0,0}, (Vector2){35, 35});
	gglTest->maxCols = 10;
	gglTest->maxRows = 6;
	gglTest->header.gravity = GUI_GRAV_CENTER_BOTTOM;
	for(int i = 0; i < 20; i++) {
		GUIImage* img = GUIImage_new(gs->gui, iconNames[i]);
		img->header.size = (Vector2){30, 30};
		GUIRegisterObject(img, gglTest);
	}
	
	GUIRegisterObject(gglTest, NULL);

	


	
	GUIScrollWindow* gsw = GUIScrollWindow_new(gs->gui);
	gsw->header.topleft = (Vector2){200, 200};
	gsw->header.size = (Vector2){100, 100};
	GUIRegisterObject(gsw, NULL);

	
	GUIValueMonitor* gfm = GUIValueMonitor_new(gs->gui, "dynamic meshes: %d", &gs->world->dmm->totalInstances, 'i');
	gfm->header.topleft = (Vector2){0,0};
	GUIRegisterObject(gfm, gsw);
	
	
	json_file_t* guijsf;
	
	guijsf = json_load_path("assets/config/main_ui.json");
	json_value_t* kids;
	json_obj_get_key(guijsf->root, "children", &kids);
	
	GUICL_LoadChildren(gs->gui, gs->gui->root, kids);
	
	GUIObject* ps = GUIObject_findChild(gs->gui->root, "perfstats");
	gt_terrain = GUIObject_findChild(ps, "terrain");
	gt_solids = GUIObject_findChild(ps, "solids");
	gt_selection = GUIObject_findChild(ps, "selection");
	gt_decals = GUIObject_findChild(ps, "decals");
	gt_emitters = GUIObject_findChild(ps, "emitters");
	gt_effects = GUIObject_findChild(ps, "effects");
	gt_lighting = GUIObject_findChild(ps, "lighting");
	gt_sunShadow = GUIObject_findChild(ps, "sunShadow");
	gt_shading = GUIObject_findChild(ps, "shading");
	gt_gui = GUIObject_findChild(ps, "gui");
	
	

	
	// commented out for hitTest testing
// 	GUIRegisterObject(gtRenderMode, NULL);
// 	GUIRegisterObject(gtSelectionDisabled, NULL);
	

	//geditTest = GUIEditNew("edit", (Vector2){.5, .5}, (Vector2){.4, .05});
	//GUIRegisterObject(geditTest, NULL);
	//InputFocusStack_PushTarget(&gs->ifs, geditTest, inputHandlers);

		
	#include "../mods/GameState_initGL.generated_thunk.c" 
	
	
	World_initGL(gs->world);
	
	
	
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
		GUIText_setString(gt_##qname, frameCounterBuf);


		query_update_gui(terrain);
		query_update_gui(solids);
		query_update_gui(selection);
		query_update_gui(decals);
		query_update_gui(effects);
		query_update_gui(emitters);
		query_update_gui(sunShadow);
		query_update_gui(shading);
		query_update_gui(lighting);
		query_update_gui(gui);
		
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
 		gs->zoom = fmin(gs->zoom, -2.0);
		gs->hasMoved = 1;
	}
	if(is->keyState[53] & IS_KEYDOWN) {
		gs->zoom -= keyZoom;
		gs->hasMoved = 1;
	}
	if(is->clickButton == 4) {
		gs->zoom += mouseZoom;
 		gs->zoom = fmin(gs->zoom, -2.0);
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
		gs->nearClipPlane = fmax(gs->nearClipPlane, 0.1);
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

	if(is->keyState[24] & IS_KEYDOWN) {
		gs->sunTheta -= 1 * te;
		gs->sunTheta = fmod(gs->sunTheta, F_2PI);
		printf("sunTheta: %f\n", gs->sunTheta);
	}
	if(is->keyState[25] & IS_KEYDOWN) {
		gs->sunTheta += 1 * te;
		gs->sunTheta = fmod(gs->sunTheta, F_2PI);
		printf("sunTheta: %f\n", gs->sunTheta);
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
			"lighting",
			"shadow depth"
		};
		
		gs->debugMode = (gs->debugMode + 1) % 7;
		lastChange = gs->frameTime;
		
		GUIText_setString(gtRenderMode, modeStrings[gs->debugMode]);
	}
	
	if(ev->keysym == XK_Insert) {
		gs->selectionPassDisabled = !gs->selectionPassDisabled;
		GUIText_setString(gtSelectionDisabled, gs->selectionPassDisabled ? "Selection Disabled" : "");
	}
	
	if(ev->character == 'b') {
		// builder control
		gbcTest = guiBuilderControlNew((Vector2){.1,.2}, (Vector2){.8,.8}, 0);
		GUIRegisterObject(gbcTest, NULL);
		guiResize(&gbcTest->header, (Vector2){.79, .79});
		guiRenderTarget_SetScreenRes(gbcTest->rt,  (Vector2i){gs->screen.wh.x, gs->screen.wh.y});
		
		InputFocusStack_PushTarget(&gs->ifs, gbcTest, inputHandlers);
	}
	
	// texture builder
	if(ev->character == 't') {
		printf("t\n");
		texbuilder = guiTexBuilderControlNew((Vector2){.15,.1}, (Vector2){.82,.82}, 0);
		GUIRegisterObject(texbuilder, NULL);
		guiResize(&texbuilder->header, (Vector2){.79, .79});
		
		InputFocusStack_PushTarget(&gs->ifs, texbuilder, inputHandlers);
	}
	
	if(ev->character == 'k') {
		RoadNetwork* rn = gs->world->roads; 
	

		if(VEC_LEN(&rn->edges) == 0) return;
		
		// choose a random road intersection
		int n = rand() % VEC_LEN(&rn->edges);
// 		printf("n: %d\n", n);
		RoadEdge* edge = VEC_ITEM(&rn->edges, n);
		RoadNode* node = edge->from;
		
		// put a tree at the intersection
		Vector v = {node->pos.x, node->pos.y, 0};
		uint32_t eid = World_spawnAt_Item(gs->world, "tree", &v);
// 		printf("adding eid: %d\n", eid);
		
		// have it wander around
		C_RoadWander crw;
		crw.speed = 7.5;
		crw.distTravelled = 0;
		crw.edge = edge;
		
		CES_addComponentName(&gs->ces, "roadWander", eid, &crw);
	


		
	}
}  


static void main_click_handler(InputEvent* ev, GameState* gs) {

	if(ev->button == 1) {
		
		// BUG: used inverse cursor pos. changed to compile temporarily
		GUIObject* hit;
		
		Vector2 pos = (Vector2){ev->intPos.x, gs->screen.wh.y - ev->intPos.y};
// 		hit = GUIManager_hitTest(gs->gui, pos);
		
		hit = GUIManager_triggerClick(gs->gui, pos);
		
		
		printf("\n\n----> %f, %f \n", ev->normPos.x, ev->normPos.y);
		if(hit) {
			printf("@@clicked in window %p %p  %f,%f\n", gs->gui->root, hit, hit->header.size.x, hit->header.size.y);
		}
		else {
			Vector2i tile;
			Vector2 invPos = {ev->normPos.x, 1 - ev->normPos.y};
			getTileFromScreenCoords(gs, ev->normPos, &tile);
			printf("x: %d, y: %d\n", tile.x, tile.y);
			Vector2 tilef = {tile.x, tile.y};
			
			
			SceneItemInfo* sii = QuadTree_findFirst(&gs->world->qt, tilef);
			if(sii) {
				printf("found item\n");
				QuadTree_purge(&gs->world->qt, sii);
			}
			
			VEC(SceneItemInfo*) dead;
			VEC_INIT(&dead);
			
			int purge(SceneItemInfo* s, void* _qt) {
				QuadTree* qt = _qt;
				VEC_PUSH(&dead, s);
				return 0;
			}
			
			QuadTree_findAllArea(&gs->world->qt, (AABB2){
				tilef.x - 10, tilef.y - 10,
				tilef.x + 10, tilef.y + 10,
			}, purge, &gs->world->qt);
			
			VEC_EACH(&dead, sii, si) {
				QuadTree_purge(&gs->world->qt, si);
			}
			
			VEC_FREE(&dead);
			
			//BUG: convert this to tile coords
// 			World_spawnAt_Item(gs->world, "gazebbq", &tilef);
			
		/*	SoundInstance* si = calloc(1, sizeof(*si));
			si->flags = 0;//SOUNDFLAG_LOOP;
			si->globalStartTime = 2.0;
			si->volume = 0.8;
		*/	//SoundManager_addClipInstance(gs->sound, "ohno", si);
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




static void main_move_handler(InputEvent* ev, GameState* gs) {
	
	Vector2i ci;
// 	Vector c;
	

	getTileFromScreenCoords(gs, ev->normPos, &ci);
	
	Vector c = {ci.x, ci.y, 0};
	Vector pos1 = {-10, -10, 20};
	Vector pos2 = {-10, 10, 20};
	Vector pos3 = {10, -10, 20};
	Vector pos4 = {10, 10, 20};
	
	vAdd(&pos1, &c, &gs->world->cursor->pos1);
	vAdd(&pos2, &c, &gs->world->cursor->pos2);
	vAdd(&pos3, &c, &gs->world->cursor->pos3);
	vAdd(&pos4, &c, &gs->world->cursor->pos4);
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
	
	mFastMul(&invp, &invv, &gs->mProjWorld);
	
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
	
	if(!gs->selectionData) {
		printf("!!! Cannot look up tile coordinates: no selection data.\n");
		return;
	}
	
	int w = (int)gs->screen.wh.x;
	int h = (int)gs->screen.wh.y;
	
	int i = (scoord.x * w) + ((scoord.y * h) * w);
	
	uint32_t j = gs->selectionData[i];
	u.in = j;
	
	gs->cursorTilePos.x = u.rgb[0];
	gs->cursorTilePos.y = u.rgb[1];
	//gs->cursorTilePos.z = u.rgb[2];
	
//	struct sGL_RG8* off = &gs->world->map.offsetData[(int)gs->cursorTilePos.z]; 
	
	int by = u.rgb[2] / 2; // set 2 to the size of the map / 256
	int bx = u.rgb[2] % 2; 
	
	//bx = by = 0;
	
	tile->x = (bx * 256.0) + gs->cursorTilePos.x;
	tile->y = (by * 256.0) + gs->cursorTilePos.y;

//	printf("*tile offset: %u - %d,%d, %d,%d %.3f,%.3f\n", j, u.rgb[0],u.rgb[1],u.rgb[2],u.rgb[3], tile->x, tile->y);
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

	
	
	// wove a window with the cursor
	//gw_test->header.topleft = (Vector2){x, h - y};
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
		gs->gui->screenSize = (Vector2i){xs->winAttr.width, xs->winAttr.height};
		
		gs->screen.aspect = gs->screen.wh.x / gs->screen.wh.y;
		
		gs->screen.resized = 1;
		
		setupFBOs(gs, 1);
		
		if(gbcTest)
			guiRenderTarget_SetScreenRes(gbcTest->rt, (Vector2i){gs->screen.wh.x, gs->screen.wh.y});
		//printf("diffuse2: %d\n",gs->diffuseTexBuffer);
	}
}



void mapRayCastHitTest(GameState* gs, Vector2 screenpos) {
	Vector ws_ray; // ray in world space
	
	// convert from screen space through ndc into world space
	Vector ss_ray = { // ray in screen space
		(screenpos.x / gs->screen.wh.x) * 2.0 - 1.0,
		(screenpos.y / gs->screen.wh.y) * 2.0 - 1.0,
		-1
	};
	
	vMatrixMul(&ss_ray, &gs->mProjWorld, &ws_ray);
	
	vNorm(&ws_ray, &ws_ray);
	
	// ws_ray is now a world-space unit vector pointing away from the mouse
	
	Vector ws_campos;
	vMatrixMul(&(Vector){0,0,0}, &gs->mProjWorld, &ws_campos);
	
	
	MapLayer* terr = gs->world->map.block->terrain;
	
	// walk along the ray projected onto the x/y plane sampling terrain height
	// start at the camera pos and continue until we hit something or fall off the map
	
	Vector groundDir;
	Plane plane = {.n = {0,1,0}, .d = 0};
	vProjectOntoPlaneNormalized(&ws_ray, &plane, &groundDir);
	
	// epsilons
	float ex = 1.0;
	float ey = 1.0;
	
	//while(1) {
		
	//	float h = Map_getTerrainHeightf(&w->map, loci);
		
	//}
	
	
}









// temp hack
void runLogic(GameState* gs, InputState* is) {
	static double spawnTimer = 0;
	return;
	RoadNetwork* rn = gs->world->roads; 
	
	spawnTimer += gs->frameSpan;
	if(spawnTimer > 6.0) {
		spawnTimer = 0;
		
		if(VEC_LEN(&rn->edges) == 0) return;
		
		// choose a random road intersection
		int n = rand() % VEC_LEN(&rn->edges);
// 		printf("n: %d\n", n);
		RoadEdge* edge = VEC_ITEM(&rn->edges, n);
		RoadNode* node = edge->from;
		
		// put a tree at the intersection
		Vector v = {node->pos.x, node->pos.y, 0};
		uint32_t eid = World_spawnAt_Item(gs->world, "tree", &v);
// 		printf("adding eid: %d\n", eid);
		
		// have it wander around
		C_RoadWander crw;
		crw.speed = 7.5;
		crw.distTravelled = 0;
		crw.edge = edge;
		
		CES_addComponentName(&gs->ces, "roadWander", eid, &crw);
	}
	
	
	
}

// temp hack
void roadWanderSystem(GameState* gs, InputState* is) {
	
	ComponentManager* posComp = CES_getCompManager(&gs->ces, "position");
	ComponentManager* wanderComp = CES_getCompManager(&gs->ces, "roadWander");
	
	CompManIter pindex, windex;
	
	
	ComponentManager_start(wanderComp, &windex);
	ComponentManager_start(posComp, &pindex);
	
	uint32_t eid = 0;
	C_RoadWander* rw;
	while(rw = ComponentManager_next(wanderComp, &windex, &eid)) { 
		Vector* pos;
		
		if(!(pos = ComponentManager_nextEnt(posComp, &pindex, eid))) {
			 continue;
		}

		rw->distTravelled = rw->distTravelled + (rw->speed * gs->frameSpan * (1 / rw->edge->length));
		
		if(rw->distTravelled >= 1.0) {
			RoadEdge* oe = rw->edge;
			RoadNode* at = rw->backwards ? rw->edge->from : rw->edge->to; 
		
			RoadEdge* e = RoadNode_GetRandomOutEdge(gs->world->roads, at);
			if(e) rw->edge = e;
			
			
			rw->backwards = e->from != at;
			//printf("\nat: %p \n", at);
			//printf("backwards %d %p %p / %p %p\n", rw->backwards, e->from, e->to, oe->from, oe->to);
			rw->distTravelled = fmod(rw->distTravelled, 1.0);
		}
		
		Vector2 p2 = RoadNetwork_Lerp(gs->world->roads, rw->edge->from, rw->edge->to, rw->distTravelled, rw->backwards);
		
		pos->x = p2.x;
		pos->y = p2.y;
		pos->z = Map_getTerrainHeightf(&gs->world->map, p2); 
		
	}
	
	
}

// temp hack
void runRelPosUpdate(GameState* gs, InputState* is) {
	
	ComponentManager* posComp = CES_getCompManager(&gs->ces, "position");
	ComponentManager* relComp = CES_getCompManager(&gs->ces, "relativeInfo");
	ComponentManager* rotComp = CES_getCompManager(&gs->ces, "rotation");
	
	CompManIter pindex, rindex, rotindex;
	CompManIter p_pindex, p_rotindex; // parents 
	
	
	ComponentManager_start(relComp, &rindex);
	ComponentManager_start(posComp, &pindex);
	ComponentManager_start(rotComp, &rotindex);

	ComponentManager_start(posComp, &p_pindex);
	ComponentManager_start(rotComp, &p_rotindex);
	
	
	uint32_t eid = 0;
	C_RelativeInfo* ri;
	while(ri = ComponentManager_next(relComp, &rindex, &eid)) { 
		
// 		printf("\nrelinfo eid: %d\n", eid);
		
		Vector* pos;
		if((pos = ComponentManager_nextEnt(posComp, &pindex, eid))) {
// 			printf(" child has pos %d\n", eid);
			Vector* p_pos;
			if((p_pos = ComponentManager_nextEnt(posComp, &p_pindex, ri->parentEID))) {
				vAdd(p_pos, &ri->pos, pos); 
// 				printf("has 2 pos %d: %f,%f,%f \n", eid, pos->x, pos->y, pos->z);
// 				printf("  parent has pos %d->%d: %f,%f,%f \n", ri->parentEID, eid, p_pos->x, p_pos->y, p_pos->z);
// 				printf("has 2 pos %d: %f,%f,%f \n", eid, ri->pos.x, ri->pos.y, ri->pos.z);
			}
		}
		
		// TODO: rotation of the parent should change the position of the child instead of rotating it too 
		
		C_Rotation* crot;
		if((crot = ComponentManager_nextEnt(rotComp, &rotindex, eid))) {
			
			C_Rotation* p_crot;
			if((p_crot = ComponentManager_nextEnt(rotComp, &p_rotindex, ri->parentEID))) {
				vAdd(&p_crot->axis, &ri->rotAxis, &crot->axis); 
				crot->theta = ri->rotTheta + p_crot->theta; 
				//printf("av: %f %f %f\n", p_crot->theta, ri->rotTheta, crot->theta);
			}
		}
		
		// todo: scale
		
		
	}
}


// temp hack
void runSystems(GameState* gs, InputState* is) {
	
	CompManIter avindex, rindex;
	// angular rotation
	ComponentManager* avComp = CES_getCompManager(&gs->ces, "angularVelocity");
	ComponentManager* rotComp = CES_getCompManager(&gs->ces, "rotation");

//	printf("####1\n");
	ComponentManager_start(avComp, &avindex);
	//printf("####2\n");
	ComponentManager_start(rotComp, &rindex);
//	printf("####3\n");
	
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
	
	
// 	CompManIter /*pindex,*/ findex;
	
	ComponentManager* posComp = CES_getCompManager(&gs->ces, "position");
	ComponentManager* pathComp = CES_getCompManager(&gs->ces, "pathFollow");
	
	CompManIter pindex, findex;
	
	
	ComponentManager_start(pathComp, &findex);
	ComponentManager_start(posComp, &pindex);
	//pindex.index = -1;
	
	eid = 0;
	C_PathFollow* pf;
	while(pf = ComponentManager_next(pathComp, &findex, &eid)) { 
	//	break;
	//	printf("eid %d \n", eid);
		Vector* pos;
		
		break; // HACK
		
		if(!(pos = ComponentManager_nextEnt(posComp, &pindex, eid))) {
	//		printf("m\n");
			 continue;
		}
		/*
		printf("%f,%f,%f\n", pos->x, pos->y, pos->z);
		pf->distTravelled += pf->speed * gs->frameSpan;
		Vector2 p2 = Path_GetPos(pf->path, pf->distTravelled);
		
		pos->x = p2.x;
		pos->y = p2.y;
		pos->z = Map_getTerrainHeightf(&gs->world->map, p2); 
		//*/
	}
	
	
	/*
	ComponentManager* mhuComp = CES_getCompManager(&gs->ces, "mapHeightUpdate");
	CompManIter hindex;
	ComponentManager_start(mhuComp, &hindex);
	ComponentManager_start(posComp, &pindex);
	
	eid = 0;
	uint8_t* up;
	while(up = ComponentManager_next(mhuComp, &hindex, &eid)) { 
		Vector* pos;
		if(!*up) continue;
		if(!(pos = ComponentManager_nextEnt(posComp, &pindex, eid))) {
			 continue;
		}
		
		//pos->z = Map_getTerrainHeight3f(&gs->world->map, *pos); 
	}
	*/
	
	roadWanderSystem(gs, is);
	
	
	// should be the last thing run
	runRelPosUpdate(gs, is);
}





#define PF_START(x) gs->perfTimes.x = getCurrentTime()
#define PF_STOP(x) gs->perfTimes.x = timeSince(gs->perfTimes.x)

void gameLoop(XStuff* xs, GameState* gs, InputState* is) {
	gs->frameCount++;
	
// 	printf("-----------frame------------\n");
	
	static erodeDelay = 0;
	//erodeDelay = (erodeDelay + 1) % 1;
	
	if(!erodeDelay) {
		
	//	MapGen_water(&gs->world->map, waterProg);
	//	MapGen_erode(&gs->world->map, erodeProg);
	
	
	//	MapGen_erode(&gs->world->map, soilProg);
	//	MapGen_erode(&gs->world->map, erodeProg);
	//	MapGen_erode(&gs->world->map, erodeProg);
	//	MapGen_erode(&gs->world->map, erodeProg);
		//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	
	}
	
	checkResize(xs,gs);
	
		PF_START(preframe);
	preFrame(gs);
		PF_STOP(preframe);
	
	InputFocusStack_DispatchPerFrame(&gs->ifs, is, gs->frameSpan);
	
	updateView(xs, gs, is);
	
	checkCursor(gs, is);
	
	
	runLogic(gs, is);
	
	runSystems(gs, is);
	
	
	drawFrame(xs, gs, is);
	
	
	gs->screen.resized = 0;

	postFrame(gs);
// 	printf("^^^^^^^^^^frame^^^^^^^^^^\n");
}
