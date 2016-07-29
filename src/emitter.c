
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


static float frand(float low, float high) {
	return low + ((high - low) * ((double)rand() / (double)RAND_MAX));
}


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




Emitter* makeEmitter() {
	
	Emitter* e;
	EmitterSprite* s;
	
	e = calloc(1, sizeof(Emitter));
	
	e->particleNum = 10;
	e->sprite = s = calloc(1, particleNum *  sizeof(EmitterSprite));
	e->instances = calloc(1, 100 * sizeof(EmitterInstance));
	
	for(i = 0; i < e->particleNum; i++) {
		s->start_pos.x = frand(-10, 10); 
		s->start_pos.y = frand(-10, 10); 
		s->start_pos.z = 20; 
		
		s->phys_fn_index = 0;
		
		s->start_vel.x = 0;
		s->start_vel.y = 0;
		s->start_vel.z = 0.01;
		
		s->spawn_delay = frand(0, 3);
		
		s->start_acc.x = 0;
		s->start_acc.y = 0;
		s->start_acc.z = 0.01;
		
		s->lifetime = 10;
		
		s->size = frand(1,3);
		s->spin = frand(-1, 1);
		s->growth_rate = frand(0, .5);
		s->randomness = frand(0, 1);
		
		s++;
	}
	
	
	return e;
}



