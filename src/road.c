 
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
#include "road.h" 


static GLuint vao, mesh_vbo, cp_vbo;
static GLuint color_ul, model_ul, view_ul, proj_ul, heightmap_ul;
static ShaderProgram* prog;
unsigned short* indices;


typedef struct RoadVertex {
	Vector2 v;
	struct { float u, v; } t;
} RoadVertex;

typedef struct RoadControlPoint {
	Vector2 cp0;
	Vector2 cp2;
	Vector2 cp1;
} RoadControlPoint;



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
	
	heightmap_ul = glGetUniformLocation(prog->id, "sHeightMap");
	
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
			
			printf("i %d  y %f\n",i, vertices[n].v.y);
		}
	}
	
	// calculate indices. the vertices run lengthwise along the road
	int n = 0;
	
	// front cap
	indices[n++] = segments * 3;
	indices[n++] = segments * 2;
	indices[n++] = segments * 1;
	indices[n++] = 0;
	
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
		{{10,10}, {128,128}, {255,255}}
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



void drawRoad(GLuint tex, Matrix* view, Matrix* proj) {
	
	Matrix model;
	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
 	mTrans3f(0,0,.05, &model);
 	mScale3f(2, 2, 2, &model);
	
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(65535);
	
	glUseProgram(prog->id);

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj->m);
	glUniform3f(color_ul, .5, .2, .9);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
	
	glUniform1i(heightmap_ul, 0);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cp_vbo);
	glexit("road vbo");
	

	
	glDrawElementsInstanced(GL_TRIANGLE_STRIP, 3 * 256 * 2 + 2 + 4 + 2, GL_UNSIGNED_SHORT, indices, 3);
	glexit("road draw");
	
}

