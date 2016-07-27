
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

#include "emitter.h"


static GLuint vao;
// static GLuint points_vbo, vbo1, vbo2;
static ShaderProgram* prog;


void initEmitters() {
	
		// VAO
	VAOConfig opts[] = {
		// per particle attributes
		{4, GL_FLOAT}, // start position & phys fn index
		{4, GL_FLOAT}, // start velocity & spawn delay
		{4, GL_FLOAT}, // start acceleration & lifetime
		{4, GL_FLOAT}, // size, angular momentum, growth rate, randomness
		
		// per emitter attributes
		{4, GL_FLOAT}, // position & scale
		{4, GL_FLOAT}, // start time, life span
		
		{0, 0}
	};
	
	vao = makeVAO(opts, 4*4 * 6);
	glexit("emitter vao");
	
	/*
	prog = loadCombinedProgram("emitter");

	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("emitter shader");
	*/
	
	
	
}


