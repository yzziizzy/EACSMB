
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

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

#include "log.h"

#include "utilities.h"
#include "config.h"
#include "shader.h"
#include "window.h"
#include "road.h"
#include "map.h"
#include "game.h"



static XStuff xs;
static GameState game;
static InputState input;


int main(int argc, char* argv[]) {
	int first = 1;
	int configStatus = 0;
	char* wd;
	
	initLog(0);
	
	// init some path info. 
	wd = getcwd(NULL, 0);
	game.dataDir = pathJoin(wd, "data");
	// temp. eventually will point to the correct save dir
	game.worldDir = pathJoin(game.dataDir, "world");
	
	free(wd);
	
	
	zeroConfig(&game.uSettings);
	configStatus = updateConfigFromFile(&game.uSettings, "defaults.ini");
	if(configStatus) {
		printf("failed to load defaults.ini [code %d]\n",configStatus);
	}
	
	configStatus = updateConfigFromFile(&game.uSettings, "settings.ini");
	if(configStatus) {
		printf("failed to load settings.ini [code %d]\n",configStatus);
	}
	
	setGameSettings(&game.settings,&game.uSettings);
	
	memset(&xs, 0, sizeof(XStuff));
	
	xs.targetMSAA = 4;
	xs.windowTitle = "EACSMB";
	
	initXWindow(&xs);
	
	while(1) {
		processEvents(&xs, &input, -1);
		
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
 			//printf("sleeptime: %f\n", sleeptime / 1000000);
			//sleeptime = 1000;
			if(sleeptime > 0) usleep(sleeptime); // problem... something is wrong in the math
		}
	}
	
	
	
	
	return 0;
}




 
 




 
 
