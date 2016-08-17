
#ifndef __EACSMB_GAME_H__
#define __EACSMB_GAME_H__

#include "config.h" // UserConfig
#include "fbo.h"

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

typedef struct QueryQueue {
	GLuint qids[6];
	int head, used;
} QueryQueue;


typedef struct GameState {
	
	GameScreen screen;
	
	GameSettings settings;
	UserConfig uSettings;
	
	GLuint diffuseTexBuffer, normalTexBuffer, depthTexBuffer, selectionTexBuffer;
	GLuint framebuffer;
	GLuint depthRenderbuffer;
	
	GLuint* fboTextures; 
	Framebuffer gbuf;
	Framebuffer selectionbuf;
	Framebuffer decalbuf;
	
	void* selectionData;
	uint64_t selectionFrame;
	GLsync selectionFence;
	GLuint selectionPBOs[2];
	char readPBO, activePBO;
	
	MapInfo map;
	
	MatrixStack view;
	MatrixStack proj;
	
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
	
	float zoom;
	float direction;
	Vector lookCenter;
	Vector2 mapCenter;
	

	
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
		
	} queries;

	
	// info for the selection pass
	char hasMoved; // if the view has moved since the last selection pass
	uint64_t lastSelectionFrame; // frame number of the last time a selection pass was rendered
	
	
	// temp stuff with no better place atm
	int activeTool;
	
	float timeOfDay; // radians of earth spin. 0 = midnight, pi/2 = morning
	// need time of year and latitude for sun angle
	Vector sunNormal;
	float sunSpeed;
	
} GameState;








// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs, InputState* is);
void gameLoop(XStuff* xs, GameState* gs, InputState* is);

void setGameSettings(GameSettings* g, UserConfig* u);



#endif // __EACSMB_GAME_H__
