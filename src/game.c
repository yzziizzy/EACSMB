
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"
#include "text/text.h"

#include "utilities.h"
#include "shader.h"
#include "window.h"
#include "game.h"




void initGame(XStuff* xs, GameState* gs) {
	
	glClearColor(0.0f, 0.6f, 0.0f, 0.0f);
	
	
	gs->tileProg = loadProgram("tiles", "tiles", "tiles", NULL, NULL, NULL);
	
}




void renderFrame(XStuff xs, GameState* gs) {
	
	
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	
	// grab uniforms
	
	// set up matrices
	
	// set up vbo's
	
	// draw "tiles"
	
	
	
	
	
	glXSwapBuffers(xs->display, xs->clientWin);
}











