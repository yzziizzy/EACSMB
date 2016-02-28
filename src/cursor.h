#ifndef __EACSMB_CURSOR_H__
#define __EACSMB_CURSOR_H__







typedef struct Selection {
	AABB box;
	GLuint vbo;
	int primCnt;
	
	
	
	
} Selection;





void initMarker();


void renderMarker(GameState* gs, int tx, int ty); 












#endif // __EACSMB_CURSOR_H__