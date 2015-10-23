


#define MAP_SIZE 128




typedef struct {
	
	ShaderProgram* tileProg;
	
	
	Vector eyePos;
	Vector eyeDir;
	
	float timeOfDay;
	Vector sunPos;
	
} GameState;








// use a normal map to not have the overlap problem




void renderFrame(XStuff* xs, GameState* gs);







