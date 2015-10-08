
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"
#include "text/text.h"

#include "utilities.h"
#include "shader.h"
#include "window.h"



XStuff xs;
InputState input;


int main(int argc, char* argv[]) {
	
	memset(&xs, 0, sizeof(XStuff));
	
	xs.targetMSAA = 4;
	xs.windowTitle = "EACSMB";
	
	initXWindow(&xs);
	
	
	while(1) {
		processEvents(&xs, &input, -1);
		
		
	}
	
	
	
	
	return 0;
}




 
 
void glexit(char* msg) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "GL ERROR: %s: %s \n", msg, gluErrorString(err));
		exit(-1);
	}
}

char* glerr(char* msg) {
	char* errstr;
	GLenum err;
	
	err = glGetError();
	errstr = NULL;
	
	if (err != GL_NO_ERROR) { 
		errstr = (char*)gluErrorString(err);
		fprintf(stderr, "GL ERROR: %s: %s \n", msg, errstr);
	}
	
	return errstr;
}



 
 
