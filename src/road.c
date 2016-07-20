 
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"

#include "utilities.h"
#include "objloader.h"
#include "shader.h"
#include "texture.h"
#include "road.h" 
#include "sim.h" 


static GLuint vao, mesh_vbo, cp_vbo;
static GLuint color_ul, model_ul, view_ul, proj_ul, heightmap_ul, heightmap_offset_ul;
static Texture* road_tex;
static GLuint screenSize_ul;
static ShaderProgram* prog;
unsigned short* indices;



void initRoads() {
	
	// VAO
	VAOConfig opts[] = {
		{4, GL_FLOAT}, // position & tex
		{4, GL_FLOAT}, // control points 0 and 2 (the ends)
		{2, GL_FLOAT}, // control point 1 (the middle)
		{0, 0}
	};
	
	vao = makeVAO(opts, 4*4 + 4*4 + 2*4);
	glexit("road vao");
	
	

	// shader
	prog = loadCombinedProgram("projRoad");
	
	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	color_ul = glGetUniformLocation(prog->id, "color");
	screenSize_ul = glGetUniformLocation(prog->id, "screenSize");
	
	heightmap_ul = glGetUniformLocation(prog->id, "sHeightMap");
	heightmap_offset_ul = glGetUniformLocation(prog->id, "sOffsetLookup");
	
	glProgramUniform1i(prog->id, heightmap_ul, 21);
	glProgramUniform1i(prog->id, heightmap_offset_ul, 20);
//	glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sDepth"), 6);
	
	road_tex = loadBitmapTexture("./assets/textures/road-256.png");


	
	glexit("road shader");
	
	// initialize a triangle strip
	int segments = 255;
	
	segments++; //the end cap
	RoadVertex* vertices = malloc(4 * segments * sizeof(RoadVertex));
	indices = malloc((20 + (2 * segments * 3)) * sizeof(unsigned short));
	
	// oriented along the y axis, with x=0 being the center and .5 units to each side
	// the entire mesh is 1 unit long
	
	// u tex coord is horizontal across the roadway.
	// v tex coord runs along the roadway
	
	// calculate vertices
	float xvals[] = {-2.0f, -1.0f, 1.0f, 2.0f}; 
	
	int i, j;
	for(j = 0; j < 4; j++) {
		for(i = 0; i < segments; i++) {
			int n = i + (j * segments);
			vertices[n].v.x = xvals[j];
			vertices[n].v.y = i * (1.0f/(float)(segments-1));
			
			vertices[n].t.u = j < 2 ? 0.0f : 1.0f;
			vertices[n].t.v = i * (1.0f/(float)(segments-1));
		}
	}
	
	// calculate indices. the vertices run lengthwise along the road
	int n = 0;
	
	// front cap
	indices[n++] = segments * 3;
	indices[n++] = segments * 2;
	
	// left leg
	for(i = 0; i < segments; i++) {
		indices[n++] = i;
		indices[n++] = i + segments;
	}
	
	// end cap
	indices[n++] = segments * 4 - 1;
	indices[n++] = segments * 3 - 1;
	
	indices[n++] = 65535;
	
	// top
	for(i = 0; i < segments; i++) {
		indices[n++] = i + segments;
		indices[n++] = i + segments * 2;
	}
	indices[n++] = 65535;

	// right leg
	for(i = 0; i < segments; i++) {
		indices[n++] = i + segments * 2;
		indices[n++] = i + segments * 3;
	}
	indices[n++] = 65535;
	
	
	// init some control point data
	RoadControlPoint cps[] = {
		{{10,10}, {255,10}, {100,10}},
		{{10,10}, {10,255}, {10,100}},
		{{10,10}, {255,255}, {128,128}}
	};
	
	
	// upload mesh data
	glexit("before road vbo load");
	glBindVertexArray(vao);
	
	glGenBuffers(1, &mesh_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	
	glEnableVertexAttribArray(0);
//	glEnableVertexAttribArray(1);
//	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*4, 0);
//	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 3*4 + 2*4 + 2*2, 3*4);
//	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 3*4 + 2*4 + 2*2, 3*4 + 2*4);

	glBufferData(GL_ARRAY_BUFFER, 4 * segments * sizeof(RoadVertex), vertices, GL_STATIC_DRAW);


	glGenBuffers(1, &cp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cp_vbo);

	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*4 + 2*4, 0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4*4 + 2*4, 4*4);

	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(RoadControlPoint), cps, GL_STATIC_DRAW);
	
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glexit("road mesh load");
	
	free(vertices);
}


RoadBlock* allocRoadBlock() {
	RoadBlock* rb = calloc(1, sizeof(RoadBlock));
	
	rb->maxRoads = 256;
	rb->cps = malloc(rb->maxRoads * sizeof(RoadControlPoint));
	
	return rb;
}

int rbAddRoad(RoadControlPoint* rcp, int* out_road_id) {
	int id 
	

void drawRoad(GLuint dtex, Matrix* view, Matrix* proj) {
	
	Matrix model;
	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
 	mTrans3f(0,0,.05, &model);
 	mScale3f(2, 2, 2, &model);
	
	// should move somewhere higher. primitive restart should probably remain enabled everywhere
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(65535);
	
	glUseProgram(prog->id);

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj->m);
	glUniform3f(color_ul, .5, .2, .9);
	
	glUniform2f(screenSize_ul, 600, 600);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cp_vbo);
	glexit("road vbo");
	
	glActiveTexture(GL_TEXTURE0 + 2);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, dtex);
	glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sDepth"), 2);
	
	glActiveTexture(GL_TEXTURE0 + 25);
	glBindTexture(GL_TEXTURE_2D, road_tex->tex_id);
	glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sRoadTex"), 25);


	//                              3 strips, 2 endcaps, 2 primitive restarts
	glDrawElementsInstanced(GL_TRIANGLE_STRIP, 3 * 256 * 2 + 2 + 2 + 2, GL_UNSIGNED_SHORT, indices, 3);
	glexit("road draw");
	
}


void roadsSyncGraph(TransGraph* tg) {
	
	
	
	
}

