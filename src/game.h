




typedef struct {
	
	ShaderProgram* tileProg;
	
	MatrixStack model;
	MatrixStack view;
	MatrixStack proj;
	
	Vector eyePos;
	Vector eyeDir;
	
	float zoom;
	float direction;
	Vector lookCenter;
	
	float timeOfDay;
	Vector sunPos;
	
	float frameTime; // ever incrementing time of the this frame
	float frameSpan; // the length of this frame, since last frame
	
	
	
} GameState;








// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs, InputState* is);







