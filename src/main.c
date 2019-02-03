
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <limits.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"

#include "log.h"

#include "utilities.h"
#include "config.h"
#include "shader.h"
#include "window.h"
#include "road.h"
#include "map.h"
#include "loadingScreen.h"
#include "game.h"


#ifndef DISABLE_SOUND
	#include "sound.h"
#endif




static XStuff xs;
static GameState game;
static InputState input;

static LoadingScreen loadingScreen;


static char* helpText;
int parseOpts(int argc, char* argv[], GlobalSettings* gs);



void meh(void* m) {
	initGame(&xs, &game);
	loadingScreen.done = 1;
// 	pthread_exit(NULL);
}


int main(int argc, char* argv[]) {
	int first = 1;
	int configStatus = 0;
	char* wd;
	
	GlobalSettings_loadDefaults(&game.globalSettings);
	
	parseOpts(argc, argv, &game.globalSettings);
	
	GlobalSettings_loadFromFile(&game.globalSettings, game.globalSettings.coreConfigPath);
	GlobalSettings_finalize(&game.globalSettings);
	
	Shader_setGlobalShaderDir(game.globalSettings.shadersDirPath);
	
	initLog(0);
	
	// init some path info. 
	wd = getcwd(NULL, 0);
	game.dataDir = pathJoin(wd, "data");
	// temp. eventually will point to the correct save dir
	game.worldDir = pathJoin(game.dataDir, "world");
	
	free(wd);
	
	input.doubleClickTime = 0.200;
	input.dragMinDist = 4;
	
	memset(&xs, 0, sizeof(XStuff));
	clearInputState(&input);
	
	xs.targetMSAA = 4;
	xs.windowTitle = "EACSMB";
	
	initXWindow(&xs);
	
	
	
	pthread_t initThread;
	pthread_create(&initThread, NULL, meh, NULL);
	//meh(NULL);
	
	// initialization progress loop
	while(!loadingScreen.done) {
		processEvents(&xs, &input, &game.ifs, -1);
		
		if(first && xs.ready) {
			LoadingScreen_init(&loadingScreen);
			
			first = 0;
		}
		
		if(xs.ready) {
			loadingScreen.resolution.x = xs.winAttr.width;
			loadingScreen.resolution.y = xs.winAttr.height;
			
			LoadingScreen_draw(&loadingScreen, &xs);
		}
		
		usleep((1.0/80.0) * 1000000);
	}

	initGameGL(&xs, &game);
	printf("init complete\n");
	
	// main running loop
	while(1) {
		processEvents(&xs, &input, &game.ifs, -1);
		
		#ifndef DISABLE_SOUND
			SoundManager_tick(game.sound, game.frameTime);
		#endif
		
		gameLoop(&xs, &game, &input);
		
		
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




 
 


int parseOpts(int argc, char* argv[], GlobalSettings* gs) {
	int an;
	
	
	for(an = 1; an < argc; an++) {
		char* arg = argv[an];
		
		if(0 == strcmp(arg, "--help")) {
			puts(helpText);
			exit(0);
		}
		
		if(0 == strcmp(arg, "-f")) {
			an++;
			gs->coreConfigPath = strdup(argv[an]);
			continue;
		}
	}
	
	return 0;
}

 
 
 
 
 
static char* helpText = "" \
	"Usage: [rtfs]\n" ;
