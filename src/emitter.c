
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






EmitterManager* EmitterManager_alloc(GlobalSettings* gs) {
	
	EmitterManager* em = pcalloc(em);
	EmitterManager_init(em, gs);
	
	return em;
}


void EmitterManager_init(EmitterManager* em, GlobalSettings* gs) {
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
	
	
	em->mdi = MultiDrawIndirect_alloc(vaoopts, gs->EmitterManager_maxInstances, "emitterManager");
	em->mdi->isIndexed = 0;
	em->mdi->primMode = GL_POINTS;
	em->mdi->uniformSetup = (void*)uniformSetup;
	em->mdi->instanceSetup = (void*)instanceSetup;
	em->mdi->data = em;
}


void EmitterManager_initGL(EmitterManager* em, GlobalSettings* gs) {
	MultiDrawIndirect_initGL(em->mdi);
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




