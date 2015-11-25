 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"
#include "window.h"
#include "map.h"
#include "game.h"



GLuint selectionVAO;
ShaderProgram* cursorProg;
Texture* cursorTex;


// void initCursors() {
// 	
// 	
// 	// shader program
// 	cursorProg = loadCombinedProgram("cursor");
// 	
// 	// uniform locations
// 	MVP_ul = glGetUniformLocation(cursorProg->id, "mMVP");
// 
// 	// vao
// 	VAOConfig opts[] = {
// 		{3, GL_FLOAT}, // position
// 		{2, GL_FLOAT}, // tex coords
// 		{0, 0}
// 	};
// 	
// 	selectionVAO = makeVAO(opts, 2*3*4);
// 	
// 	
// 	// texture
// 	cursorTex = loadBitmapTexture("./assets/textures/cursorFire.png");
// 
// 	
// }



//
void updateCursorMesh(AABB* box) {
	
	
	
	
}





// void renderCursor(GameState* gs) {
// 	
// 	// fiddle with matrices and variables
// 	
// 	
// 	// activate program
// 	glUseProgram(cursorProg->id);
// 	glexit("");
// 	
// 	// set uniforms
// 	glUniformMatrix4fv(model_ul, 1, GL_FALSE, mModel->m);
// 	
// 	// draw elements
// 	
// 	
// }













ShaderProgram* markerProg;
GLuint markerVAO, markerVBO;
GLuint markerMVP_ul, markerColor_ul;
Mesh* markerMesh;

void initMarker() {
	
	// program
	markerProg = loadCombinedProgram("marker");
	
	// uniform locations
	markerMVP_ul = glGetUniformLocation(markerProg->id, "mMVP");	
	markerColor_ul = glGetUniformLocation(markerProg->id, "color");	
	
	// VBO
	glGenBuffers(1, &markerVBO);
	glBindBuffer(GL_ARRAY_BUFFER, markerVBO);
	
	// VAO
	VAOConfig opts[] = {
		{3, GL_FLOAT}, // position
		{0, 0}
	};
	
	markerVAO = makeVAO(opts, 3*4);
	
	
	// mesh data
	Matrix mat = IDENT_MATRIX;
	markerMesh = makeCube(&mat, 1);
	
	
	glBufferData(GL_ARRAY_BUFFER, markerMesh->vertexCnt * sizeof(Vector), markerMesh->vertices, GL_STATIC_DRAW);
	glerr("");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}


void renderMarker(GameState* gs, int tx, int ty) {
	
	// fiddle with matrices and variables
	// get world coordinates and height for the tile
	
	Matrix m;
	mIdent(&m);
	
	// activate program
	glUseProgram(markerProg->id);
	glexit("");
	
	mMul(msGetTop(&gs->proj), &m);
	mMul(msGetTop(&gs->view), &m);
	mTrans3f(512, 512, 70, &m);  
	mScale3f(40,40,40, &m);
	
	
	// set uniforms
	glUniformMatrix4fv(markerMVP_ul, 1, GL_FALSE, m.m);
	glUniform4f(markerColor_ul, .6, .8, .2, 1.0);
	
	// bind vbo
	glBindVertexArray(markerVAO);
	glBindBuffer(GL_ARRAY_BUFFER, markerVBO);
	
	// draw elements
	glDrawElements(GL_TRIANGLES, 3*8, GL_UNSIGNED_SHORT, markerMesh->indices);
	
}









