
#ifndef __EACSMB_GAME_H__
#define __EACSMB_GAME_H__

#include "common_math.h"
#include "common_gl.h"

#include "settings.h"
#include "uniformBuffer.h"
#include "fbo.h"
#include "scene.h"
#include "world.h"
#include "shadowMap.h"
#include "window.h"
#include "gui.h"


#include "component.h"

#ifndef DISABLE_SOUND
	#include "sound.h"
#endif


typedef struct GameScreen {
	
	float aspect;
	Vector2 wh;
	
	int resized;
	
} GameScreen;


typedef struct GameSettings {
	
	float keyRotate;
	float keyScroll;
	float keyZoom;
	
	float mouseRotate;
	float mouseScroll;
	float mouseZoom;
	
} GameSettings;




typedef struct PerViewUniforms {
	Matrix view;
	Matrix proj;
} PerViewUniforms;


typedef struct PerFrameUniforms {
	float wholeSeconds;
	float fracSeconds;
} PerFrameUniforms;


typedef struct GameState {
	
	char* dataDir;
	char* worldDir;
	
	GameScreen screen;
	
	GameSettings settings;
	GlobalSettings globalSettings;
	
	GLuint diffuseTexBuffer, normalTexBuffer, depthTexBuffer, selectionTexBuffer, lightingTexBuffer;
	GLuint framebuffer;
	GLuint depthRenderbuffer;
	
	GLuint* fboTextures; 
	Framebuffer gbuf;
	Framebuffer selectionbuf;
	Framebuffer decalbuf;
	Framebuffer lightingbuf;
	
	uint32_t* selectionData;
	uint64_t selectionFrame;
	GLsync selectionFence;
	GLuint selectionPBOs[2];
	char readPBO, activePBO;
	
	Scene scene;
	World* world;
	
	GUIManager* gui;
	RenderPass* guiPass;
	
	UniformBuffer perViewUB;
	UniformBuffer perFrameUB;
	
	MatrixStack view;
	MatrixStack proj;
	
	Matrix invView; // TODO: rename these
	Matrix invProj;
	Matrix mProjWorld;
	
	double nearClipPlane;
	double farClipPlane;
	
	Vector eyePos;
	Vector eyeDir;
	Vector eyeUp;
	Vector eyeRight;
	
	Vector cursorTilePos;
	Vector2 cursorPos;
// 	Vector cursorPos;
	int cursorIndex;

	Vector2 mouseDownPos;
	
	int debugMode;
	int show_qt_debug;
	
	float zoom;
	float direction;
	Vector lookCenter;
	Vector2 mapCenter;
	

	InputFocusStack ifs;
	InputEventHandler* defaultInputHandlers;
	
	double frameTime; // ever incrementing time of the this frame
	double frameSpan; // the length of this frame, since last frame
	uint64_t frameCount; // ever incrementing count of the number of frames processed
	
	// performance counters
	struct {
		double preframe;
		double selection;
		double draw;
		double decal;
		double light;
		double shade;
		
	} perfTimes;
	
	struct {
		QueryQueue draw; 
		QueryQueue terrain; 
		QueryQueue solids; 
		QueryQueue selection; 
		QueryQueue decals; 
		QueryQueue emitters; 
		QueryQueue effects; 
		QueryQueue lighting; 
		QueryQueue sunShadow; 
		QueryQueue shading; 
		QueryQueue gui; 
		
	} queries;

	
	// info for the selection pass
	char hasMoved; // if the view has moved since the last selection pass
	uint64_t lastSelectionFrame; // frame number of the last time a selection pass was rendered
	char selectionPassDisabled;
	
	// temp stuff with no better place atm
	int activeTool;
	
	float timeOfDay; // radians of earth spin. 0 = midnight, pi/2 = morning
	// need time of year and latitude for sun angle
	Vector sunNormal;
	float sunSpeed;
	float sunTheta;
	
	
	
	CES ces;
	
#ifndef DISABLE_SOUND
	SoundManager* sound;
#endif
	
	#include "../mods/GameState.generated_mixin.h" 
	
} GameState;




void initGame(XStuff* xs, GameState* gs);
void initGameGL(XStuff* xs, GameState* gs);
void getTileFromScreenCoords(GameState* gs, Vector2 scoord, Vector2i* tile);

void setupFBOs(GameState* gs, int resized);


// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs, InputState* is, PassFrameParams* pfp);
void gameLoop(XStuff* xs, GameState* gs, InputState* is);


void initRenderLoop(GameState* gs);


void depthPrepass(XStuff* xs, GameState* gs, InputState* is);




#endif // __EACSMB_GAME_H__
