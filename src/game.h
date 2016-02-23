




typedef struct GameState {
	
	Vector2 viewWH;
	
	GLuint diffuseTexBuffer, normalTexBuffer, depthTexBuffer, selectionTexBuffer;
	GLuint framebuffer;
	GLuint depthRenderbuffer;
	
	MapInfo map;
	
	MatrixStack view;
	MatrixStack proj;
	
	Vector eyePos;
	Vector eyeDir;
	
	Vector2 cursorPos;
	
	Vector2 mouseDownPos;
	
	int debugMode;
	
	float zoom;
	float direction;
	Vector lookCenter;
	Vector2 mapCenter;
	
	float timeOfDay; // radians of earth spin. 0 = midnight, pi/2 = morning
	// need time of year for sun angle
	Vector sunNormal;
	float sunSpeed;
	
	double frameTime; // ever incrementing time of the this frame
	double frameSpan; // the length of this frame, since last frame
	
	int activeTool;
	
} GameState;








// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs, InputState* is);
void gameLoop(XStuff* xs, GameState* gs, InputState* is);






