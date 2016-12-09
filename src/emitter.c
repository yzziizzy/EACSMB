
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
static GLuint tex_ul;
static Texture* sprite_tex;

static GLuint ubo_alignment;

static GLuint frame_ubo;
static int next_ubo_region = 0;
static GLuint ubo_fences[3];
static float* ubo_ptr; 

static float frand(float low, float high) {
	return low + ((high - low) * ((double)rand() / (double)RAND_MAX));
}


// terrible code, but use for now
int waitSync(GLuint id) {
	GLenum ret;
	if(!id || !glIsSync(id)) return 1;
	while(1) {
		ret = glClientWaitSync(id, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		glexit("");
		if(ret == GL_ALREADY_SIGNALED || ret == GL_CONDITION_SATISFIED)
		  return 0;
	}
}


static void* getUBORegionPointer() {
	// the fence at index n protects from writing to index n.
	// it is set after commands for n - 1;
	waitSync(ubo_fences[next_ubo_region]);
	
	return &ubo_ptr[next_ubo_region * ubo_alignment / 4];
}

static setUBOFence() {
	
	if(ubo_fences[next_ubo_region]) glDeleteSync(ubo_fences[next_ubo_region]);
	glexit("");
	ubo_fences[next_ubo_region] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glexit("");
	
	next_ubo_region = (next_ubo_region + 1) % 3; // BUG: make sure this is the right one
	
}

void initEmitters() {
	
	GLbitfield flags;
	size_t ubo_size;
	
	GLuint block_index;
	
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &ubo_alignment);
	
	next_ubo_region = 0;
	ubo_fences[0] = 0;
	ubo_fences[1] = 0;
	ubo_fences[2] = 0;
	
	flags =  GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
	ubo_size = ubo_alignment * 3;
	
	glGenBuffers(1, &frame_ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, frame_ubo);
	glBufferStorage(GL_UNIFORM_BUFFER, ubo_size, NULL, flags);
	glexit("ubo storage");
	
	ubo_ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, ubo_size, flags);
	glexit("ubo persistent map");
	
	
	sprite_tex = loadBitmapTexture("./assets/textures/terrible_smoke.png");
		// VAO
	VAOConfig opts[] = {
		// per particle attributes
		{4, GL_FLOAT}, // start position & offset
		{4, GL_FLOAT}, // start velocity & spawn delay
		{4, GL_FLOAT}, // start acceleration & lifetime
		{4, GL_FLOAT}, // size, angular momentum, growth rate, randomness
		{4, GL_FLOAT}, // fade-in, fade-out, unallocated
		
		// per emitter attributes
		{4, GL_FLOAT}, // position & scale
		{4, GL_FLOAT}, // start time, life span
		
		{0, 0}
	};
	
	vao = makeVAO(opts);
	glexit("emitter vao");
	
	
	prog = loadCombinedProgram("emitter");

//	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
// 	timeS_ul = glGetUniformLocation(prog->id, "timeSeconds");
// 	timeMS_ul = glGetUniformLocation(prog->id, "timeFractional");
	
	tex_ul = glGetUniformLocation(prog->id, "textures");
//	color_ul = glGetUniformLocation(prog->id, "color");

	block_index = glGetUniformBlockIndex(prog->id, "globalTimer");
	glexit("");
	glUniformBlockBinding(prog->id, block_index, 0);
	glexit("");
	
	glexit("emitter shader");
}




Emitter* makeEmitter() {
	
	int i;
	Emitter* e;
	EmitterSprite* s;
	
	e = calloc(1, sizeof(Emitter));
	
	e->particleNum = 100;
	e->sprite = s = calloc(1, e->particleNum *  sizeof(EmitterSprite));
	e->instances = ar_alloc(e->instances, 200);
	
	for(i = 0; i < e->particleNum; i++) {
		s->start_pos.x = frand(-.5, .5); 
		s->start_pos.y = frand(-.5, .5); 
		s->start_pos.z = frand(0, 1); 
		
		s->offset = frand(0, 2);
		
		s->start_vel.x = 0;
		s->start_vel.y = 0;
		s->start_vel.z = 1.5;
		
		s->spawn_delay = frand(0, 6);
		
		s->start_acc.x = frand(-0.3, 0.3);
		s->start_acc.y = frand(-0.3, 0.3);
		s->start_acc.z = 0.5;
		
		s->lifetime = 7;
		
		s->size = frand(2,6);
		s->spin = frand(-1, 1);
		s->growth_rate = frand(0, .4);
		s->randomness = frand(0, 1);
		
		s->fade_in = frand(1, 2);
		s->fade_out = frand(2, 4);
		
		s++;
	}


	// upload sprite data
	glexit("before emitter sprite");
	glBindVertexArray(vao);
	
	glGenBuffers(1, &e->instance_vbo);
	glGenBuffers(1, &e->points_vbo);

	glBindBuffer(GL_ARRAY_BUFFER, e->points_vbo);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*4*5, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*4*5, 1*4*4);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4*4*5, 2*4*4);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4*4*5, 3*4*4);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4*4*5, 4*4*4);

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

	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 2*4*4, 0*4*4);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 2*4*4, 1*4*4);

	printf("instance pos %d %f %f %f \n",  sizeof(EmitterInstance), e->instances[0].pos.x, e->instances[0].pos.y, e->instances[0].pos.z);
	glBufferData(GL_ARRAY_BUFFER, e->instanceNum * sizeof(EmitterInstance), e->instances, GL_STATIC_DRAW);
	
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glexit("emitter sprite load");
}


void Draw_Emitter(Emitter* e, Matrix* view, Matrix* proj, double time) {
		
	Matrix model;
	
	
	glUseProgram(prog->id);
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDepthMask(GL_FALSE); // disable depth writes for paritcles


	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj->m);
	
	double seconds = (float)(long)time;
	double milliseconds = time - seconds;
	
	float* tb = getUBORegionPointer();

	tb[0] = seconds;
	tb[1] = milliseconds;
	//glUniform1f(timeS_ul, seconds); // TODO figure out how to fix rounding nicely
	//glUniform1f(timeMS_ul, milliseconds); // TODO figure out how to fix rounding nicely

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, frame_ubo, next_ubo_region*ubo_alignment, 4*2);
	glexit("");
//	glUniform3f(color_ul, .5, .2, .9);
// 	glUniform2f(screenSize_ul, 600, 600);

	glActiveTexture(GL_TEXTURE0 + 24);
	glBindTexture(GL_TEXTURE_2D, sprite_tex->tex_id);
	glProgramUniform1i(prog->id, tex_ul, 24);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, e->points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, e->instance_vbo);
	glexit("emitter vbo");

	//printf("num, inst: %d, %d\n", e->particleNum, e->instanceNum);                         
	glDrawArraysInstanced(GL_POINTS, 0, e->particleNum, e->instanceNum);
	glexit("emitter draw");
	
	setUBOFence();
	
	glDepthMask(GL_TRUE);
}
