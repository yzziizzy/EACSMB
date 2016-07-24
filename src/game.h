
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


typedef struct GameState {
	
	GameScreen screen;
	
	GameSettings settings;
	UserConfig uSettings;
	
	GLuint diffuseTexBuffer, normalTexBuffer, depthTexBuffer, selectionTexBuffer;
	GLuint framebuffer;
	GLuint depthRenderbuffer;
	
	GLuint* fboTextures; 
	Framebuffer gbuf;
	Framebuffer decalbuf;
	
	MapInfo map;
	
	MatrixStack view;
	MatrixStack proj;
	
	double nearClipPlane;
	double farClipPlane;
	
	Vector eyePos;
	Vector eyeDir;
	
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
	
	float timeOfDay; // radians of earth spin. 0 = midnight, pi/2 = morning
	// need time of year and latitude for sun angle
	Vector sunNormal;
	float sunSpeed;
	
	double frameTime; // ever incrementing time of the this frame
	double frameSpan; // the length of this frame, since last frame
		
	struct {
		double preframe;
		double selection;
		double draw;
		double decal;
		double light;
		double shade;
		
	} perfTimes;
	
	int activeTool;
	
} GameState;








// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs, InputState* is);
void gameLoop(XStuff* xs, GameState* gs, InputState* is);

void setGameSettings(GameSettings* g, UserConfig* u);



#endif // __EACSMB_GAME_H__
