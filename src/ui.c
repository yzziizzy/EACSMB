
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

ShaderProgram* windowProg;
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



void initUI() {
	
	
	msAlloc(20, &uiMat);
	msIdent(&uiMat);
	msOrtho(0, 600, 0, 600, -1, 100, &uiMat);
	
	
	initRootWin();
	windowProg = loadCombinedProgram("ui");
	
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
	
	
	
	
}



// shitty temporary function for rendering ui
void renderWindowTmp(float x, float y, int index) {
	
	
	msPush(&uiMat);
	
	msTrans3f(x, y, 0, &uiMat);
	
	msScale3f(50, 50, 50, &uiMat);
	


	glUniform1i(glGetUniformLocation(windowProg->id, "texIndex"), index);
	glexit("");
	
	GLuint zz = glGetUniformLocation(windowProg->id, "mMVP");
	glexit("");
	glUniformMatrix4fv(zz, 1, GL_FALSE, msGetTop(&uiMat));
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
	// set uniforms
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D_ARRAY, icons->tex_id);
	glexit("");

	glUniform1i(glGetUniformLocation(windowProg->id, "sTexture"), 3);
	glexit("");
	
	// mess with matrices
	
	renderWindowTmp(550, 10, 0);
	renderWindowTmp(550, 60, 1);
	renderWindowTmp(550, 110, 2);
	

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


