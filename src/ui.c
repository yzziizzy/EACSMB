
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "text/text.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"
#include "ui.h"



UIWindow uiRootWin;


void handleClick(int button, Vector2 pos) {
	
	
	
	
	
}



void initRootWin() {
	uiRootWin.box.min.x = 0.0;
	uiRootWin.box.min.y = 0.0;
	uiRootWin.box.max.x = 1.0;
	uiRootWin.box.max.y = 1.0;
	
	uiRootWin.z = 0;
	
	uiRootWin.type = 0;
	
	uiRootWin.parent = NULL;
	uiRootWin.kids = NULL;
	
}


void renderWindow(UIWin* win, MatrixStack* base) {
	
	
	
}





