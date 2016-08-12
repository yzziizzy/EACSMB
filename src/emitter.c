
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

static GLuint model_ul, view_ul, proj_ul, timeS_ul, timeMS_ul;




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
	
	
	prog = loadCombinedProgram("emitter");

//	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	timeS_ul = glGetUniformLocation(prog->id, "timeSeconds");
	timeMS_ul = glGetUniformLocation(prog->id, "timeFractional");
//	color_ul = glGetUniformLocation(prog->id, "color");

	
	glexit("emitter shader");
}




Emitter* makeEmitter() {
	
	int i;
	Emitter* e;
	EmitterSprite* s;
	
	e = calloc(1, sizeof(Emitter));
	
	e->particleNum = 10;
	e->sprite = s = calloc(1, e->particleNum *  sizeof(EmitterSprite));
	e->instances = ar_alloc(e->instances, 100);
	
	for(i = 0; i < e->particleNum; i++) {
		s->start_pos.x = frand(-1, 1); 
		s->start_pos.y = frand(-1, 1); 
		s->start_pos.z = frand(0, 1); 
		
		s->phys_fn_index = 0;
		
		s->start_vel.x = 0;
		s->start_vel.y = 0;
		s->start_vel.z = 1.5;
		
		s->spawn_delay = frand(0, 5);
		
		s->start_acc.x = 0;
		s->start_acc.y = 0;
		s->start_acc.z = 0.5;
		
		s->lifetime = 5;
		
		s->size = frand(1,3);
		s->spin = frand(-1, 1);
		s->growth_rate = frand(0, .4);
		s->randomness = frand(0, 1);
		
		s++;
	}


	// upload sprite data
	glexit("before emitter sprite");
	glBindVertexArray(vao);
	
	glGenBuffers(1, &e->instance_vbo);
	glGenBuffers(1, &e->points_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, e->points_vbo);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*4*4, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*4*4, 1*4*4);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4*4*4, 2*4*4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4*4*4, 3*4*4);

	glBufferData(GL_ARRAY_BUFFER, e->particleNum * sizeof(EmitterSprite), e->sprite, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glexit("emitter sprite load");
	
	
	
	return e;
}


// ei is copied internally
void emitterAddInstance(Emitter* e, EmitterInstance* ei) {
	//return;
	printf("out instance here %d\n", ar_info(e->instances)->next_index );
	if(ar_hasRoom(e->instances, 1)) {
		printf("in instance here\n");
		ar_append_direct(e->instances, *ei);
		e->instanceNum++;
	}
	
	
}

void emitter_update_vbo(Emitter* e) {
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, e->instance_vbo);

	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 2*4*4, 0*4*4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 2*4*4, 1*4*4);

	printf("instance pos %d %f %f %f \n",  sizeof(EmitterInstance), e->instances[0].pos.x, e->instances[0].pos.y, e->instances[0].pos.z);
	glBufferData(GL_ARRAY_BUFFER, e->instanceNum * sizeof(EmitterInstance), e->instances, GL_STATIC_DRAW);
	
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glexit("emitter sprite load");
}


void Draw_Emitter(Emitter* e, Matrix* view, Matrix* proj, double time) {
		
	Matrix model;
	
	glUseProgram(prog->id);

	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj->m);
	
	double seconds = (float)(long)time;
	double milliseconds = time - seconds;
	
	glUniform1f(timeS_ul, seconds); // TODO figure out how to fix rounding nicely
	glUniform1f(timeMS_ul, milliseconds); // TODO figure out how to fix rounding nicely

//	glUniform3f(color_ul, .5, .2, .9);
// 	glUniform2f(screenSize_ul, 600, 600);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, e->points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, e->instance_vbo);
	glexit("emitter vbo");

/*	
	glActiveTexture(GL_TEXTURE0 + 2);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, dtex);
	glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sDepth"), 2);
	
	glActiveTexture(GL_TEXTURE0 + 25);
	glBindTexture(GL_TEXTURE_2D, road_tex->tex_id);
	glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sRoadTex"), 25);
*/

	//printf("num, inst: %d, %d\n", e->particleNum, e->instanceNum);                         
	glDrawArraysInstanced(GL_POINTS, 0, e->particleNum, e->instanceNum);
	glexit("emitter draw");
	
	
	
}