
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include "c3dlas/c3dlas.h"
#include "c3dlas/meshgen.h"

#include "utilities.h"
#include "shader.h"
#include "texture.h"

#include "lighting.h"


static GLuint vao;
// static GLuint points_vbo, vbo1, vbo2;
static ShaderProgram* prog;

static GLuint model_ul, view_ul, proj_ul, timeS_ul, timeMS_ul;



#define PC_BUFFER_DEPTH 16



// the distance at which the given light falls below an 8 bit integer's precision
float lightDistance(float constant, float linear, float quadratic) {
	return -linear + sqrt((linear * linear) + 4 * quadratic * (constant - 256)) / (2 * quadratic);	
}


static void preFrame(PassFrameParams* pfp, LightManager* lm);
static void draw(LightManager* lm, PassDrawable* pd, PassDrawParams* pdp);
static void postFrame(LightManager* lm);





void initLighting() {
	
	
	// VAO
	VAOConfig opts[] = {
		// per vertex
		{0, 4, GL_FLOAT, 0, GL_FALSE}, // position, category
		
		// per instance 
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // position, constant intensity
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // direction, linear intensity
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // color, quadratic intensity
		{1, 4, GL_FLOAT, 1, GL_FALSE}, // cutoff angle, exponent, unused, unused
		
		{0, 0, 0}
	};
	
	vao = makeVAO(opts);

	glexit("dynamic mesh vao");
	
	// shader
	prog = loadCombinedProgram("lighting");
	
	view_ul = glGetUniformLocation(prog->id, "mWorldView");
	proj_ul = glGetUniformLocation(prog->id, "mViewProj");
	
	glexit("dynamic mesh shader");
	
	
}

int TRIS = 0;

void LightManager_Init(LightManager* lm) {
	
	// geometry. 
	// all light geometry is fixed at game start.
	
	int i;
	size_t offset;
	
	// regular icosahedron
	Vector4 icos_points[] = {
		{ 1.0f, 0.0f, F_GOLDEN, 1}, // top edge along x axis
		{ -1.0f, 0.0f, F_GOLDEN, 1},
		
		{ 0.0f, F_GOLDEN, 1.0f, 1}, // triangle tips off that top edge
		{ 0.0f, -F_GOLDEN, 1.0f, 1},
		
		{ F_GOLDEN, 1.0f, 0.0f, 1}, // waistline
		{ F_GOLDEN, -1.0f, 0.0f, 1},
		{ -F_GOLDEN, 1.0f, 0.0f, 1},
		{ -F_GOLDEN, -1.0f, 0.0f, 1},
		
		{ 0.0f, F_GOLDEN, -1.0f, 1}, // triangle tips off the bottom edge
		{ 0.0f, -F_GOLDEN, -1.0f, 1},
		
		{ 1.0f, 0.0f, -F_GOLDEN, 1}, // bottom edge along x axis
		{ -1.0f, 0.0f, -F_GOLDEN, 1}
	};
	
	TRIS = 3 * 20;
	unsigned short icos_indices[] = {
		// top two triangles
		0, 2, 1, 
		0, 1, 3,
		
		// neighbors of those top two triangles
		// signs are shared in x and y
		2, 0, 4,
		2, 6, 1,
		3, 5, 0,
		3, 1, 7,
		
		// standing triangle end caps
		0, 5, 4,
		1, 6, 7,
		
		// sideways triangles
		// all y values share sign
		3, 9, 5,
		3, 7, 9,
		2, 4, 8,
		2, 8, 6,
	
		// ---- triangles are correctly wound above here, below is random
	
		// inverted triangle end caps
		10, 5, 6,
		11, 7, 8,
		
		// neighbors of the two bottom triangles
		// signs are shared in x and y
		8, 10, 4,
		8, 11, 6,
		9, 10, 5,
		9, 11, 7,
		
		// bottom two triangles
		10, 8, 11, 
		11, 10, 9 
	};
	
	lm->maxInstances = 1024 * 50;
	
	//geometry vbo
	glBindVertexArray(vao);
	
	if(glIsBuffer(lm->geomVBO)) glDeleteBuffers(1, &lm->geomVBO);
	glGenBuffers(1, &lm->geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, lm->geomVBO);
	

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 1*4*4, 0);

	glBufferStorage(GL_ARRAY_BUFFER, sizeof(icos_points), NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
		
	void* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	memcpy(buf, icos_points, sizeof(icos_points));
			
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
	// geometry ibo
	glGenBuffers(1, &lm->geomIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lm->geomIBO);
	
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(icos_indices), NULL, GL_MAP_WRITE_BIT);

	uint16_t* ib = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		
	memcpy(ib, icos_indices, sizeof(icos_indices));

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
	
	
	glBindVertexArray(vao);
	// instances
	PCBuffer_startInit(&lm->instVB, lm->maxInstances * sizeof(LightInstance), GL_ARRAY_BUFFER);
	glexit("");
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glexit("");
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*4*4, (void*)(0));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4*4*4, (void*)(1*4*4));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4*4*4, (void*)(2*4*4));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4*4*4, (void*)(3*4*4));
	glexit("");
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glexit("");
	PCBuffer_finishInit(&lm->instVB);
	glexit("");
	
	PCBuffer_startInit(
		&lm->indirectCmds, 
		PC_BUFFER_DEPTH * sizeof(DrawArraysIndirectCommand), 
		GL_DRAW_INDIRECT_BUFFER
	);
	PCBuffer_finishInit(&lm->indirectCmds);
glexit("");
}


void LightManager_updateLights(LightManager* lm) {
	int i;
	
	LightInstance* vmem = PCBuffer_beginWrite(&lm->instVB);
	
	if(!vmem) {
		printf("attempted to update invalid light manager\n");
		return;
	}
//printf("num lights %d\n", VEC_LEN(&lm->lights));
	// TODO make instances switch per frame
	for(i = 0; i < VEC_LEN(&lm->lights); i++) {
		LightInstance* li = &VEC_ITEM(&lm->lights, i);
				
		// only write sequentially. random access is very bad.
		*vmem = *li;
		vmem++;
	}

}



void LightManager_AddPointLight(LightManager* lm, Vector pos, float radius, float intensity) {
	LightInstance* l;
	
	VEC_INC(&lm->lights);
	l = &VEC_TAIL(&lm->lights);
	
	l->pos = pos;
	l->radius = radius;
	
}






RenderPass* LightManager_CreateRenderPass(LightManager* lm) {
	
	RenderPass* rp;
	PassDrawable* pd;
	
	pd = Pass_allocDrawable("LightManager");
	pd->data = lm;
	pd->preFrame = preFrame;
	pd->draw = (PassDrawFn)draw;
	pd->postFrame = postFrame;
	pd->prog = prog;
	
	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}












static void preFrame(PassFrameParams* pfp, LightManager* lm) {
	
	//dynamicMeshManager_updateMatrices(mm);
	LightManager_updateLights(lm);
	
	// set up the indirect draw commands
	DrawElementsIndirectCommand* cmds = PCBuffer_beginWrite(&lm->indirectCmds);
	
	int index_offset = 0;
	int instance_offset = 0;
	int mesh_index;
	for(mesh_index = 0; mesh_index < 1; mesh_index++) {
		//DynamicMesh* dm = VEC_ITEM(&lm->meshes, mesh_index);
			
		cmds[mesh_index].firstIndex = 0;//index_offset; // offset of this mesh into the instances
		cmds[mesh_index].count = TRIS;//dm->indexCnt; // number of polys
		
		// offset into instanced vertex attributes 
		cmds[mesh_index].baseInstance = (lm->maxInstances * ((lm->instVB.nextRegion) % PC_BUFFER_DEPTH)) + instance_offset; 
		// number of instances
		cmds[mesh_index].instanceCount = VEC_LEN(&lm->lights); 
		
		
		cmds[mesh_index].baseVertex = 0;
		
		//index_offset += 20 * 3;//dm->indexCnt;// * sizeof(DynamicMeshVertex);//dm->indexCnt;
		//instance_offset += VEC_LEN(&dm->instances[0]);
		
	}

}

// this one has to handle different views, such as shadow mapping and reflections

static void draw(LightManager* lm, PassDrawable* pd, PassDrawParams* pdp) {
	

	glUseProgram(prog->id);
	glexit("");

	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, pdp->mViewProj->m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, pdp->mWorldView->m);
	
	
	glActiveTexture(GL_TEXTURE0 + 23);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, lm->dtex);
	glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sDepth"), 23);
	
	
	// ---------------------------------
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, lm->geomVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lm->geomIBO);
	
	PCBuffer_bind(&lm->instVB);
	PCBuffer_bind(&lm->indirectCmds);
	
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0, 1/* number of light types/meshes VEC_LEN(&lm->lightmeshes*/, 0);
	glexit("multidrawarraysindirect");

}



static void postFrame(LightManager* lm) {
	PCBuffer_afterDraw(&lm->instVB);
	PCBuffer_afterDraw(&lm->indirectCmds);
}











