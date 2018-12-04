#ifndef __EACSMB_loadingScreen_h__
#define __EACSMB_loadingScreen_h__


#include "common_gl.h"
#include "common_math.h"


#include "shader.h"
#include "window.h"



typedef struct {
	
	Vector2 resolution;
	float time;
	
	volatile int done;
	
	// rendering
	ShaderProgram* prog;
	GLuint quadVAO, quadVBO;
	
	GLuint ul_timer;
	GLuint ul_resolution;
	
} LoadingScreen;






void LoadingScreen_init(LoadingScreen* ls);
void LoadingScreen_draw(LoadingScreen* ls, XStuff* xs);



#endif // __EACSMB_loadingScreen_h__
