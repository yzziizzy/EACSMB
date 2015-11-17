
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

char* iconFiles[] = {
	"./assets/ui/icons/res.png",
	"./assets/ui/icons/com.png",
	"./assets/ui/icons/ind.png",
	NULL
};
	
void initRootWin();



void initUI() {
	
	initRootWin();
	windowProg = loadCombinedProgram("ui");
	
	// uniform locations

	
	icons = loadTexArray(iconFiles);
	
	// VAO
	glGenVertexArrays(1, &windowVAO);
	glBindVertexArray(windowVAO);

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WindowVertex), 0);
	
	// texture coords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(WindowVertex), (void*)offsetof(WindowVertex, u));
	
	// texture index
	glEnableVertexAttribArray(2);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(WindowVertex), (void*)offsetof(WindowVertex, texIndex));
	
	

	
	
	
	float vertices[] = {
		-1.0, -1.0, 0.0,
		-1.0, 1.0, 0.0,
		1.0, -1.0, 0.0,
		1.0, 1.0, 0.0
	};

	glGenBuffers(1, &windowVBO);
	glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
/*	
	UIIcon* resIcon = malloc(sizeof(UIIcon));
	UIIcon* comIcon = malloc(sizeof(UIIcon));
	UIIcon* indIcon = malloc(sizeof(UIIcon));
	
	resIcon->texID = resTex->tex_id;
	comIcon->texID = comTex->tex_id;
	indIcon->texID = indTex->tex_id;
	
	resIcon->win.dims.x = 128;
	resIcon->win.dims.y = 128;
	comIcon->win.dims.x = 128;
	comIcon->win.dims.y = 128;
	indIcon->win.dims.x = 128;
	indIcon->win.dims.y = 128;
	
	resIcon->win.pos.x = .8;
	resIcon->win.pos.y = .8;*/
	
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


/*

The ui is drawn on the actual framebuffer.

*/
void renderUI(XStuff* xs, GameState* gs) {
	
	
	// mess with matrices
	Matrix world;
	
	world = IDENT_MATRIX;
	
	// set uniforms
	// activate vbo's
	
	
	// draw geometry
	glBindVertexArray(windowVAO);
	glBindBuffer(GL_ARRAY_BUFFER, windowVBO);
	glexit("ui vbo");
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glexit("ui draw");
	
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


