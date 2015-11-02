




typedef struct {
	
	float viewW, viewH;
	
	GLuint diffuseTexBuffer, normalTexBuffer, depthTexBuffer;
	GLuint framebuffer;
	GLuint depthRenderbuffer;
	
	TerrainBlock* terrain;
	
	MatrixStack model;
	MatrixStack view;
	MatrixStack proj;
	
	Vector eyePos;
	Vector eyeDir;
	
	int debugMode;
	
	float zoom;
	float direction;
	Vector lookCenter;
	
	float timeOfDay; // radians of earth spin. 0 = midnight, pi/2 = morning
	// need time of year for sun angle
	Vector sunPos;
	
	double frameTime; // ever incrementing time of the this frame
	double frameSpan; // the length of this frame, since last frame
	
	
	
} GameState;








// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs, InputState* is);
void gameLoop(XStuff* xs, GameState* gs, InputState* is);






