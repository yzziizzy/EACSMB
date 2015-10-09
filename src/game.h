


#define MAP_SIZE 128




typedef struct {
	
	ShaderProgram* tileProg;
	
	
	Vector eyePos;
	Vector eyeDir;
	
	float timeOfDay;
	Vector sunPos;
	
} GameState;





typedef struct {
	unsigned int info;
	float height;
} MapTile;



typedef struct {
	int x,y;
	Vector2 center;
	
	MapTile tiles[MAP_SIZE][MAP_SIZE];
	
} MapNode;




void renderFrame(XStuff xs, GameState* gs);





