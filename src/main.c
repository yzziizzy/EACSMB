
 
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
#include "map.h"
#include "game.h"



XStuff xs;
GameState game;
InputState input;

Vector2i viewWH = {
	.x = 0,
	.y = 0
};

int main(int argc, char* argv[]) {
	int first = 1;
	
	memset(&xs, 0, sizeof(XStuff));
	
	xs.targetMSAA = 4;
	xs.windowTitle = "EACSMB";
	
	initXWindow(&xs);
	
	game.viewWH.x = 0;
	game.viewWH.y = 0;
	
	while(1) {
		processEvents(&xs, &input, -1);
		
		// should probably move into processEvents,
		// may need to have a way of checking first
		// to do that, before calling resizeUI
		if (!first && xs.ready && (
			viewWH.x != xs.winAttr.width
			|| viewWH.y != xs.winAttr.height
		)) {
			viewWH.x = xs.winAttr.width;
			viewWH.y = xs.winAttr.height;
			
			game.viewWH.x = (float)xs.winAttr.width;
			game.viewWH.y = (float)xs.winAttr.height;
			
			resizeUI(&game);
			
			printf("resetting view dimensions\n");
		}
		
		if (first && xs.ready) {
			initGame(&xs, &game);
			first = 0;
		}
		
		if(xs.ready) {
			
			
			gameLoop(&xs, &game, &input);
		}
		
		
		if(game.frameSpan < 1.0/60.0) {
			// shitty estimation based on my machine's heuristics, needs improvement
			float sleeptime = (((1.0/60.0) * 1000000) - (game.frameSpan * 1000000)) * .7;
// 			printf("sleeptime: %f\n", sleeptime / 1000000);
			if(sleeptime > 0) usleep(sleeptime); // problem... something is wrong in the math
		}
	}
	
	
	
	
	return 0;
}




 
 




 
 
