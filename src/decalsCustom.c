
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"


#include "decalsCustom.h"

#include "utilities.h"
#include "objloader.h"
#include "shader.h"
#include "pass.h"
#include "component.h"

#include "c_json/json.h"


static GLuint vao, geomVBO, geomIBO;
static GLuint view_ul, proj_ul;
static ShaderProgram* prog;




static void preFrame(PassFrameParams* pfp, CustomDecalManager* dm);
static void draw(CustomDecalManager* dm, GLuint progID, PassDrawParams* pdp);
static void postFrame(CustomDecalManager* dm);


static void sortDecalRenderOrder(CustomDecalManager* dm);



static VAOConfig vaoConfig[] = {
	// per vertex
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // position
	{0, 2, GL_UNSIGNED_SHORT, 0, GL_FALSE}, // tex
	
	// per instance 
	{1, 4, GL_FLOAT, 1, GL_FALSE}, // pos1, thickness
	{1, 4, GL_FLOAT, 1, GL_FALSE}, // pos2, alpha, unused1/2
	{1, 4, GL_FLOAT, 1, GL_FALSE}, // pos3, unused1
	{1, 4, GL_FLOAT, 1, GL_FALSE}, // pos4, unused2
	{1, 2, GL_UNSIGNED_SHORT, 1, GL_FALSE}, // tex index and tiling info
	
	{0, 0, 0}
};



void initCustomDecals() {
	
	PassDrawable* pd;
	
	// VAO
	vao = makeVAO(vaoConfig);
	glexit("custom decals vao");
	
	// shader
	prog = loadCombinedProgram("decalsCustom");
	
	//model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mWorldView");
	proj_ul = glGetUniformLocation(prog->id, "mViewProj");
	//color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("custom decals shader");

	
	// global decal geometry box
	CustomDecalVertex box_vertices[] = {
		//// front face
		//{{-.5, -.5,  .5}, {0,0}},
		//{{-.5,  .5,  .5}, {0,0}},
		//{{ .5,  .5,  .5}, {0,0}},
		//{{ .5, -.5,  .5}, {0,0}},
		//// back face
		//{{-.5, -.5, -.5}, {0,0}},
		//{{-.5,  .5, -.5}, {0,0}},
		//{{ .5,  .5, -.5}, {0,0}},
		//{{ .5, -.5, -.5}, {0,0}},
		// front face
		{{-.5, -.5,  .5}, {3,0}},
		{{-.5,  .5,  .5}, {2,0}},
		{{ .5,  .5,  .5}, {0,0}},
		{{ .5, -.5,  .5}, {1,0}},
		// back face
		{{-.5, -.5, -.5}, {3+4,0}},
		{{-.5,  .5, -.5}, {2+4,0}},
		{{ .5,  .5, -.5}, {0+4,0}},
		{{ .5, -.5, -.5}, {1+4,0}},
	};
	
	/*
	0 - iiiii
	1 - iii
	2 - iiiii
	3 - iiiii
	4 - iiiiii
	5 - iiii
	6 - iiiii
	7 - iii
	*/
	
	unsigned short box_indices[] = {
		4,6,5, 4,7,6, // -z
		0,1,2, 0,2,3, // +z
		0,4,1, 1,4,5, // -x
		2,6,3, 3,6,7, // +x
		0,3,4, 3,7,4, // -y
		1,5,2, 2,5,6, // +y
	};
	
	glBindVertexArray(vao);
	
	if(glIsBuffer(geomVBO)) glDeleteBuffers(1, &geomVBO);
	glGenBuffers(1, &geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, geomVBO);
	updateVAO(0, vaoConfig);

// 	glEnableVertexAttribArray(0);
// 	glEnableVertexAttribArray(1);
// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 1*3*4 + 4, 0);
// 	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, 1*3*4 + 4, 1*3*4);

	glBufferStorage(GL_ARRAY_BUFFER, sizeof(box_vertices), NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	void* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	memcpy(buf, box_vertices, sizeof(box_vertices));
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	

	// global decal box gemotry index buffer	
	if(glIsBuffer(geomIBO)) glDeleteBuffers(1, &geomIBO);
	glGenBuffers(1, &geomIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geomIBO);
	
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(box_indices), NULL, GL_MAP_WRITE_BIT);

	uint16_t* ib = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	memcpy(ib, box_indices, sizeof(box_indices));
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
	
	
	//pd = Pass_allocDrawable("decal manager");
	//pd->preFrame = preFrame;
	//pd->draw = draw;
	//pd->postFrame = postFrame;
	//pd->prog = prog;
	
	//Pass_registerDrawable(pd);
}




// rendering overhead
RenderPass* CustomDecalManager_CreateRenderPass(CustomDecalManager* lm) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = CustomDecalManager_CreateDrawable(lm);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* CustomDecalManager_CreateDrawable(CustomDecalManager* lm) {
	PassDrawable* pd;

	pd = Pass_allocDrawable("CustomDecalManager");
	pd->data = lm;
	pd->preFrame = preFrame;
	pd->draw = (PassDrawFn)draw;
	pd->postFrame = postFrame;
	pd->prog = prog;
	
	return pd;
}




CustomDecalManager*  CustomDecalManager_alloc(GlobalSettings* gs) {
	CustomDecalManager* dm;
	
	dm = calloc(1, sizeof(*dm));
	CustomDecalManager_init(dm, gs);
	
	return dm;
}


void CustomDecalManager_init(CustomDecalManager* dm, GlobalSettings* gs) {
	VEC_INIT(&dm->decals);
	VEC_INIT(&dm->renderOrder);
	HT_init(&dm->lookup, 6);
	
	dm->maxInstances = gs->CustomDecalManager_maxInstances;
	
// 	dm->instPool = MemPool_alloc(sizeof(CustomDecalInstance), 1024 * 64);
}


void CustomDecalManager_initGL(CustomDecalManager* dm, GlobalSettings* gs) {
	glBindVertexArray(vao);
	
	// per-instance attributes
	int stride = calcVAOStride(1, vaoConfig);
	
	PCBuffer_startInit(&dm->instVB, dm->maxInstances * stride, GL_ARRAY_BUFFER);
	updateVAO(1, vaoConfig);
	PCBuffer_finishInit(&dm->instVB);
	
	
	PCBuffer_startInit(
		&dm->indirectCmds, 
		16 * sizeof(DrawArraysIndirectCommand), 
		GL_DRAW_INDIRECT_BUFFER
	);
	PCBuffer_finishInit(&dm->indirectCmds);
}




// makes a decal of a certain width from one spot to another
// returns the index of the instance
int CustomDecalManager_AddInstanceStrip(CustomDecalManager* dm, int index, Vector start, Vector end, float width) {
	
	CustomDecalInstance di;
	
	return CustomDecalManager_AddInstance(dm, index, &di);
}


// returns the index of the instance
CustomDecalInstance* CustomDecalManager_AddInstance(CustomDecalManager* dm, int index, const CustomDecalInstance* di) {
	
	CustomDecal* d; 
	CustomDecalInstance* ldi;
	
	if(index >= VEC_LEN(&dm->decals)) {
		fprintf(stderr, "custom decal manager addInstance out of bounds: %d, %d\n", (int)VEC_LEN(&dm->decals), index);
		return -1;
	}
	
	dm->totalInstances++;
	
	//printf("adding instance: %d ", meshIndex);
	d = VEC_ITEM(&dm->decals, index);
	
	VECMP_INSERT(&d->instances, *di);
	ldi = VECMP_LAST_INSERT(&d->instances);
	ldi->thickness = d->thickness;
	ldi->texIndex = d->texIndex;
	printf("texindex: %d\n", d->texIndex);
	//VEC_PUSH(&d->instances[1], *di);
	//VEC_INC(&d->instMatrices);
	
	//printf("add instance: %d", mm->totalInstances, VEC_LEN(&msh->instances[0]));
	
	return ldi;
}

void CustomDecalManager_DelInstance(CustomDecalManager* dm, CustomDecalInstance* di) {
	VEC_EACH(&dm->decals, i, d) {
		if(VECMP_OWNS_PTR(&d->instances, di)) {
			VECMP_DELETE(&d->instances, di);
			return;
		}
	}
}


/*
CustomDecalInstance* CustomDecalManager_AddEphInstance(CustomDecalManager* dm, int index, const CustomDecalInstance* di) {
	CustomDecalInstance* l_cdi;
	CustomDecal* d; 
	
	d = VEC_ITEM(&dm->decals, index);
	
	l_cdi = MemPool_malloc(dm->instPool);
	if(di) *l_cdi = *di;
	l_cdi->thickness = d->thickness;
	l_cdi->texIndex = d->texIndex;
	
	VEC_PUSH(&d->ephInstances, l_cdi);
	
	dm->totalInstances++;
	
	return l_cdi;
}

void CustomDecalManager_DelEphInstance(CustomDecalManager* dm, CustomDecalInstance* di) {
	
	// shit. fix later.
	VEC_EACH(&dm->decals, i1, d) {
		VEC_EACH(&d->ephInstances, i2, ldi) {
			VEC_RM(&d->ephInstances, i2);
			dm->totalInstances--;
			MemPool_free(dm->instPool, di);
			return;
		}
	}
	
}*/

// returns the index of the instance
int CustomDecalManager_lookupName(CustomDecalManager* dm, char* name) {
	
	int64_t index;
	
	if(!HT_get(&dm->lookup, name, &index)) {
		//printf("decal found: %s -> %d\n", name, index);
		return index;
	}
	
	printf("decal not found: %s\n", name);
	return -1;
}


// returns the index if the decal
int CustomDecalManager_AddDecal(CustomDecalManager* dm, char* name, CustomDecal* d) {
	int index;
	
	VECMP_INIT(&d->instances, 1024 * 1024);
	
	VEC_PUSH(&dm->decals, d);
	//mm->totalVertices += m->vertexCnt;
	//mm->totalIndices += sm->indexCnt;
	index = VEC_LEN(&dm->decals);
	
	VEC_PUSH(&dm->renderOrder, index - 1);
	sortDecalRenderOrder(dm);
	
	HT_set(&dm->lookup, name, index -1);
	
	return index - 1;
}




void CustomDecalManager_updateMatrices(CustomDecalManager* dm, PassFrameParams* pfp) {
	int decal_index, ro_index, i;
	
	
	
	CustomDecalInstance* vmem = PCBuffer_beginWrite(&dm->instVB);
	
	if(!vmem) {
		printf("attempted to update invalid dynamic mesh manager\n");
		return;
	}

	
	for(ro_index = 0; ro_index < VEC_LEN(&dm->renderOrder); ro_index++) {
		decal_index = VEC_ITEM(&dm->renderOrder, ro_index);
		CustomDecal* d = VEC_ITEM(&dm->decals, decal_index);
		d->numToDraw = 0;
		
		// normal instances are intended to have long lifespans
		
		// TODO make instances switch per frame
		VECMP_EACH(&d->instances, i, di) {
// 			printf("cdi: %d\n", di->texIndex);
			float dist = vDist(&di->pos1, &pfp->dp->eyePos);
				
		//	printf("d %f -- ", d);
		//	printf("%f, %f, %f -- ", di->pos.x, di->pos.y, di->pos.z);
		//	printf("%f, %f, %f\n", pfp->dp->eyePos.x, pfp->dp->eyePos.y, pfp->dp->eyePos.z);
			
			//if(dist > 500) continue;
			d->numToDraw++;
			
			// only write sequentially. random access is very bad.
			*vmem = *di;
			
			vmem++;
		}
/*
		// ephemeral instances are temporary by design and used 
		// for things like cursors and user feedback
		VEC_EACH(&d->ephInstances, i, di) {
			// ephemeral instances of custom decal are always drawn. 
			d->numToDraw++;
			
			// only write sequentially. random access is very bad.
			*vmem = *di;
			vmem++;
		}
		*/
		//vmem += VEC_LEN(&dm->instances);
		//printf("num to draw %d\n", d->numToDraw);
	}
	
}








// -------------------------------------------------------------------


static void preFrame(PassFrameParams* pfp, CustomDecalManager* dm) {
	
	CustomDecalManager_updateMatrices(dm, pfp);

	
	
	DrawElementsIndirectCommand* cmds = PCBuffer_beginWrite(&dm->indirectCmds);
	
	int index_offset = 0;
	int instance_offset = 0;
	int ro_index;
	for(ro_index = 0; ro_index < VEC_LEN(&dm->renderOrder); ro_index++) {
		int decal_index = VEC_ITEM(&dm->renderOrder, ro_index);
		CustomDecal* d = VEC_ITEM(&dm->decals, decal_index);
			
		cmds[ro_index].firstIndex = index_offset; // offset of this mesh into the instances
		cmds[ro_index].count = 36;//dm->indexCnt; // number of polys
		
		// offset into instanced vertex attributes
		cmds[ro_index].baseInstance = (dm->maxInstances * ((dm->instVB.nextRegion) % PC_BUFFER_DEPTH)) + instance_offset; 
		// number of instances
		cmds[ro_index].instanceCount = d->numToDraw; //VEC_LEN(&dm->instances[0]); 
	//printf("instances %d %d %d %d  \n", mesh_index, dm->indexCnt, VEC_LEN(&dm->instances[0]), instance_offset );
		cmds[ro_index].baseVertex = 0;
		
		//printf("instances %d %d \n", ro_index, d->numToDraw );
		
		// index_offset += 36;// decals only have one mesh
		// TODO BUG what? seems wrong
		instance_offset += VECMP_LEN(&d->instances);
	}
}


void draw(CustomDecalManager* dm, GLuint progID, PassDrawParams* pdp) {
	
	GLuint tex_ul;

glexit("");
	glUseProgram(prog->id);
	glexit("");

	
	// TODO: move to intermediate initialization stage
	glActiveTexture(GL_TEXTURE0 + 9);
glexit("");
	glBindTexture(GL_TEXTURE_2D_ARRAY, dm->tm->tex_id);
	//printf("tex id: %d\n", dm->tm->tex_id);
glexit("");
	
	tex_ul = glGetUniformLocation(prog->id, "sTexture");
glexit("");
	glProgramUniform1i(prog->id, tex_ul, 9);
	glexit("");


	glActiveTexture(GL_TEXTURE0 + 23);
	glexit("shading tex 5");
	glBindTexture(GL_TEXTURE_2D, dm->dtex);
	glProgramUniform1i(prog->id, glGetUniformLocation(prog->id, "sDepth"), 23);
	

	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &pdp->mWorldView->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &pdp->mViewProj->m);
	
	

	//printf("instance count %d, %d\n", cmds[0].instanceCount, cmds[0].count);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, geomVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geomIBO);
	
	PCBuffer_bind(&dm->instVB);
	PCBuffer_bind(&dm->indirectCmds);
	
	// there's just one mesh for decals atm
 	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0, 1, /*VEC_LEN(&dm->decals),*/ 0);
	glexit("multidrawelementsindirect");
	
}



static void postFrame(CustomDecalManager* dm) {
	PCBuffer_afterDraw(&dm->instVB);
	PCBuffer_afterDraw(&dm->indirectCmds);
}












static int ro_comp(const void* aa, const void * bb, void* ctx) {
	int a = *((int*)aa);
	int b = *((int*)bb);
	CustomDecalManager* dm = (CustomDecalManager*)ctx;
	
	return VEC_ITEM(&dm->decals, b)->renderWeight - VEC_ITEM(&dm->decals, a)->renderWeight;
}

static void sortDecalRenderOrder(CustomDecalManager* dm) {
	VEC_SORT_R(&dm->renderOrder, ro_comp, dm);
}
