
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




static void uniformSetup(EmitterManager* em, GLuint progID);
static void instanceSetup(EmitterManager* em, EmitterInstance* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp);

static ShaderProgram* prog;







static GLuint vao;
// static GLuint points_vbo, vbo1, vbo2;


static GLuint model_ul, view_ul, proj_ul, timeS_ul, timeMS_ul;
static GLuint tex_ul;
static Texture* sprite_tex;

static GLuint ubo_alignment;

static GLuint frame_ubo;
static int next_ubo_region = 0;
static GLuint ubo_fences[3];
static float* ubo_ptr; 






EmitterManager* EmitterManager_alloc(int maxInstances) {
	
	EmitterManager* em = pcalloc(em);
	
	//em->maxInstances = maxInstances;
	VEC_INIT(&em->emitters);
	HT_init(&em->lookup, 4);
	
	static VAOConfig vaoopts[] = {
		// per particle attributes
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // start position & offset
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // start velocity & spawn delay
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // start acceleration & lifetime
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // size, angular momentum, growth rate, randomness
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // fade-in, fade-out, unallocated
		
		// per emitter attributes
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // position & scale
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // start time, life span
		
		{0, 0, 0}
	};

	
	
	em->mdi = MultiDrawIndirect_alloc(vaoopts, maxInstances);
	em->mdi->isIndexed = 0;
	em->mdi->primMode = GL_POINTS;
	em->mdi->uniformSetup = (void*)uniformSetup;
	em->mdi->instanceSetup = (void*)instanceSetup;
	em->mdi->data = em;
	
	return em;
}



int EmitterManager_addEmitter(EmitterManager* em, Emitter* e, char* name) {
	
	int index;
	MDIDrawInfo* di;
	
	
	// TODO: fill in emitter info
	
	
	di = pcalloc(di);
	
	*di = (MDIDrawInfo){
		.vertices = VEC_DATA(&e->sprites),
		.vertexCount = VEC_LEN(&e->sprites),
	};
	
	MultiDrawIndirect_addMesh(em->mdi, di);
	
	VEC_PUSH(&em->emitters, e);
	index = VEC_LEN(&em->emitters);
	
	HT_set(&em->lookup, name, index - 1);
}


void EmitterManager_addInstance(EmitterManager* em, int index, EmitterInstance* inst) {
	Emitter* e;
	
	e = VEC_ITEM(&em->emitters, index);
	VEC_PUSH(&e->instances, *inst);
}



void EmitterManager_updateGeometry(EmitterManager* em) {
	MultiDrawIndirect_updateGeometry(em->mdi);
}



// returns the index of the emitter
int EmitterManager_lookupName(EmitterManager* em, char* name) {
	
	int64_t index;
	
	if(!HT_get(&em->lookup, name, &index)) {
		printf("emitter found: %s -> %d\n", name, index);
		return index;
	}
	printf("emitter not found: %s\n", name);
	return -1;
}







static void instanceSetup(EmitterManager* em, EmitterInstance* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp) {
	int j;
	
	//diCount = 1;
	for(j = 0; j < diCount; j++) {
		Emitter* e = VEC_ITEM(&em->emitters, j);
		di[j]->numToDraw = VEC_LEN(&e->instances);
		
		VEC_EACH(&e->instances, i, inst) {
			*vmem = inst;
			vmem++;
		}
		
	//	printf("emitter numtodraw: %d\n", di[j]->numToDraw);
		di++;
	}
}

static void uniformSetup(EmitterManager* em, GLuint progID) {
	// matrices and uniforms
	GLuint tex_ul;
	

	glActiveTexture(GL_TEXTURE0 + 24);
	glBindTexture(GL_TEXTURE_2D_ARRAY, em->tm->tex_id);
	
	tex_ul = glGetUniformLocation(progID, "textures");
	glProgramUniform1i(progID, tex_ul, 24);
	glexit("");
}







RenderPass* EmitterManager_CreateRenderPass(EmitterManager* em) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = EmitterManager_CreateDrawable(em);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* EmitterManager_CreateDrawable(EmitterManager* em) {
	
	if(!prog) {
		prog = loadCombinedProgram("emitter");
		glexit("");
	}
	
	return MultiDrawIndirect_CreateDrawable(em->mdi, prog);
}







/////////////// old ////////////////////////////




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
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // start position & offset
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // start velocity & spawn delay
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // start acceleration & lifetime
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // size, angular momentum, growth rate, randomness
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // fade-in, fade-out, unallocated
		
		// per emitter attributes
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // position & scale
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // start time, life span
		
		{0, 0, 0}
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
	VEC_INIT(&e->sprites);
	VEC_INIT(&e->instances);
	
	for(i = 0; i < e->particleNum; i++) {
		VEC_INC(&e->sprites);
		s = &VEC_ITEM(&e->sprites, i);
		
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

	glBufferData(GL_ARRAY_BUFFER, VEC_LEN(&e->sprites) * sizeof(*VEC_DATA(&e->sprites)), VEC_DATA(&e->sprites), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glexit("emitter sprite load");
	
	
	
	return e;
}


// ei is copied internally
void emitterAddInstance(Emitter* e, EmitterInstance* ei) {

	VEC_PUSH(&e->instances, *ei);
	e->instanceNum++;
}

void emitter_update_vbo(Emitter* e) {
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, e->instance_vbo);

	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 2*4*4, 0*4*4);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 2*4*4, 1*4*4);

	//printf("instance pos %d %f %f %f \n",  sizeof(EmitterInstance), e->instances[0].pos.x, e->instances[0].pos.y, e->instances[0].pos.z);
	glBufferData(GL_ARRAY_BUFFER, VEC_LEN(&e->instances) * sizeof(*VEC_DATA(&e->instances)), VEC_DATA(&e->instances), GL_STATIC_DRAW);
	
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glexit("emitter sprite load");
}


























static void preFrame(PassFrameParams* pfp, Emitter* e) {

}


static void draw(Emitter* e, GLuint progID, PassDrawParams* pdp) {
	
	Matrix model;
	
	
	glUseProgram(prog->id);
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glDepthMask(GL_FALSE); // disable depth writes for paritcles


	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &pdp->mWorldView->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &pdp->mViewProj->m);
	
	
	float* tb = getUBORegionPointer();

	tb[0] = pdp->timeSeconds;
	tb[1] = pdp->timeFractional;
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
	glDrawArraysInstanced(GL_POINTS, 0, VEC_LEN(&e->sprites), VEC_LEN(&e->instances));
	glexit("emitter draw");
	
	setUBOFence();
	
	glDepthMask(GL_TRUE);
}

static void postFrame(Emitter* e) {

}



/*
RenderPass* Emitter_CreateRenderPass(Emitter* e) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = Emitter_CreateDrawable(e);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* Emitter_CreateDrawable(Emitter* e) {
	PassDrawable* pd;

	pd = Pass_allocDrawable("Emitter");
	pd->data = e;
	pd->preFrame = preFrame;
	pd->draw = (PassDrawFn)draw;
	pd->postFrame = postFrame;
	pd->prog = prog;
	
	return pd;
}
*/
