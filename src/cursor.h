#ifndef __cursor_h__
#define __cursor_h__







typedef struct Selection {
	AABB box;
	GLuint vbo;
	int primCnt;
	
	
	
	
} Selection;





void initMarker();


void renderMarker(GameState* gs, int tx, int ty); 












#endif // __cursor_h__