
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <dirent.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "text/text.h"

#include "utilities.h"
#include "map.h"
#include "shader.h"
#include "window.h"
#include "game.h"
#include "texture.h"
#include "ui.h"



UIWindow uiRootWin;

ShaderProgram* windowProg, *windowDepthProg;
GLuint windowVAO;
GLuint windowVBO;
TexArray* icons;

MatrixStack uiMat;

char* iconFiles[] = {
	"./assets/ui/icons/res.png",
	"./assets/ui/icons/com.png",
	"./assets/ui/icons/ind.png",
	NULL
};
	
void initRootWin();



void initUI(GameState* gs) {
	
	
	msAlloc(20, &uiMat);
	
	resizeUI(gs);
	
	
	initRootWin();
	windowProg = loadCombinedProgram("ui");
	// windowDepthProg = loadCombinedProgram("uiDepth");
	
	// uniform locations

	
	icons = loadTexArray(iconFiles);
	
	// VAO
	glGenVertexArrays(1, &windowVAO);
	glBindVertexArray(windowVAO);

	
// 	// texture index
// 	glEnableVertexAttribArray(2);
// 	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(WindowVertex), (void*)offsetof(WindowVertex, texIndex));
	
	

	
	
	
	float vertices[] = {
		 0.0,  0.0, 0.0,  0,0,
		 0.0,  1.0, 0.0,  0,1,
		 1.0,  0.0, 0.0,  1,0,
		 1.0,  1.0, 0.0,  1,1
	};

	glGenBuffers(1, &windowVBO);
	glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
	
	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WindowVertex), 0);
	
	// texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(WindowVertex), (void*)offsetof(WindowVertex, u));


	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
	
}

void resizeUI(GameState* gs) {
	msIdent(&uiMat);
	msOrtho(0, gs->viewWH.x, 0, gs->viewWH.y, -1, 100.0f, &uiMat);
}


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

// temporary, for now, ish
void uiPreRenderSetup() {
	
	glUseProgram(windowProg->id);
	glexit("ui prog");
	// use program
	// activate vao
	// set up textures
	
	
}


void renderUIPicking(XStuff* xs, GameState* gs) {
	
	// glUseProgram(windowDepthProg->id);
	// glexit("");
	
	
	
}



// shitty temporary function for rendering ui
void renderWindowTmp(float x, float y, int index, float glow) {
	
	
	msPush(&uiMat);
	
	msTrans3f(x, y, 0, &uiMat);
	msScale3f(50, 50, 50, &uiMat);
	
	
	glUniform1i(glGetUniformLocation(windowProg->id, "texIndex"), index);
	glexit("");
	
	glUniform1f(glGetUniformLocation(windowProg->id, "glow"), glow);
	glexit("");
	
	GLuint zz = glGetUniformLocation(windowProg->id, "mMVP");
	glexit("");
	glUniformMatrix4fv(zz, 1, GL_FALSE, (float*)msGetTop(&uiMat));
	glexit("");
	
	// activate vbo's
	glBindVertexArray(windowVAO);
	glexit("");
	glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
	glexit("ui vbo");
	
	// draw geometry
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("ui draw");
	
	
	msPop(&uiMat);
}


/*

The ui is drawn on the actual framebuffer.

*/
void renderUI(XStuff* xs, GameState* gs) {
	
	glUseProgram(windowProg->id);
	
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, icons->tex_id);
	glexit("");
	
	glUniform1i(glGetUniformLocation(windowProg->id, "sTexture"), 3);
	glexit("");
	
	// something odd is going on here with resizing, ui disappears if window is
	// resized larger, but squishes on resize smaller
	renderWindowTmp(gs->viewWH.x - 50, 10, 2, gs->activeTool == 2 ? .5 : 0);
	renderWindowTmp(gs->viewWH.x - 50, 60, 0, gs->activeTool == 0 ? .5 : 0);
	renderWindowTmp(gs->viewWH.x - 50, 110, 1, gs->activeTool == 1 ? .5 : 0);
	
}


void loadIcons(char* dirPath) {
	
	DIR* dir;
	struct dirent* de;
	int texCount = 0;
	int maxW = 0, maxH = 0;
	BitmapRGBA8* bmp;
	
	dir = opendir(dirPath);
	
	while((de = readdir(dir)) != NULL) {
		if(!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
		
		bmp = readPNG(de->d_name);
		
	}
	
	closedir(dir);
}


