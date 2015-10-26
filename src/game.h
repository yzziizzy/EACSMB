


#define MAP_SIZE 128




typedef struct {
	
	ShaderProgram* tileProg;
	
	MatrixStack model;
	MatrixStack view;
	MatrixStack proj;
	
	Vector eyePos;
	Vector eyeDir;
	
	float timeOfDay;
	Vector sunPos;
	
	float frameTime;
	
	
} GameState;








// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs);







