
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
#include "pass.h"
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
	
	GLuint diffuseTexBuffer, normalTexBuffer, materialTexBuffer, depthTexBuffer, lightingTexBuffer;
	GLuint framebuffer;
	GLuint depthRenderbuffer;
	
	GLuint* fboTextures; 
	Framebuffer gbuf;
	Framebuffer decalbuf;
	Framebuffer lightingbuf;
	
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
	Matrix mWorldProj;
	Matrix mProjWorld;
	
	double nearClipPlane;
	double farClipPlane;
	
	Vector eyePos;
	Vector eyeDir;
	Vector eyeUp;
	Vector eyeRight;
	
	PassFrameParams debugCamPFP;
	char use_debugCam;
	char refresh_debugCam;
	
	Vector cursorTilePos;
	Vector2 cursorPos;
// 	Vector cursorPos;i
	int cursorIndex;
	
	char hasMoved;

	Vector2 mouseDownPos;
	
	int debugMode;
	char show_qt_debug;
	char show_debugWireframe;
	
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
		double draw;
		double decal;
		double light;
		double shade;
		
	} perfTimes;
	
	struct {
		QueryQueue draw; 
		QueryQueue terrain; 
		QueryQueue solids; 
		QueryQueue decals; 
		QueryQueue leaves; 
		QueryQueue emitters; 
		QueryQueue effects; 
		QueryQueue lighting; 
		QueryQueue sunShadow; 
		QueryQueue shading; 
		QueryQueue gui; 
		
	} queries;

	struct {
		int leaves: 1; 
		int decals : 1; 
		int solids : 1; 
		int effects : 1; 
		int emitters : 1; 
		int terrain : 1; 
		int lighting : 1; 
		int shadows : 1; 
	} enableDraw;
	
	// temp stuff with no better place atm
	int activeTool;
	
	float timeOfDay; // radians of earth spin. 0 = midnight, pi/2 = morning
	// need time of year and latitude for sun angle
	Vector sunNormal;
	float sunSpeed;
	float sunTheta;
	
	
	char takeScreenShot;
	
	
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


void ray_from_screen(GameState* gs, Vector2 screenPos, Vector* origin, Vector* ray);
void ray_from_screeni(GameState* gs, Vector2i screenPos, Vector* origin, Vector* ray);

#endif // __EACSMB_GAME_H__
