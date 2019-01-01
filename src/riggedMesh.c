
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"


#include "riggedMesh.h"
#include "component.h"

#include "utilities.h"
#include "objloader.h"
#include "shader.h"
#include "pass.h"


#include "c_json/json.h"






static GLuint vao;
static GLuint color_ul, model_ul, view_ul, proj_ul;
static ShaderProgram* prog;
Texture* tex;


// new
static void uniformSetup(RiggedMeshManager* mm, GLuint progID);
static void instanceSetup(RiggedMeshManager* mm, RiggedMeshInstShader* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp);





// VAO
VAOConfig vao_opts[] = {
	// per vertex
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // position
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // normal
	{0, 2, GL_UNSIGNED_SHORT, 0, GL_TRUE}, // tex
	{0, 4, GL_UNSIGNED_BYTE, 0, GL_FALSE }, // bone indices, 1-4
	{0, 4, GL_FLOAT, 0, GL_FALSE }, // bone weights, 1-4
	
	// per instance 
	{1, 1, GL_MATRIX_EXT, 1, GL_FALSE}, // model-world matrix
	{1, 2, GL_UNSIGNED_SHORT, 1, GL_FALSE}, // texture indices
	{1, 1, GL_UNSIGNED_LONG, 1, GL_FALSE}, // bone buffer offset
	
	{0, 0, 0}
};





void initRiggedMeshes() {
	
	PassDrawable* pd;
	

	
//	vao = makeVAO(vao_opts);

	glexit("Rigged mesh vao");
	
	// shader
	prog = loadCombinedProgram("riggedMeshInstanced");
	
	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	color_ul = glGetUniformLocation(prog->id, "color");
	

	
	glexit("Rigged mesh shader");
	
	//tex = loadBitmapTexture("./assets/textures/gazebo-small.png");
	
	//glActiveTexture(GL_TEXTURE0 + 8);
	//glBindTexture(GL_TEXTURE_2D, tex->tex_id);
	
	glexit("");
	

}




RiggedMesh* RiggedMeshFromOBJ(OBJContents* obj) {
	
	int i;
	RiggedMesh* m;
	
	
	m = calloc(1, sizeof(*m));
	
	m->vertexCnt = 3 * obj->faceCnt;
	m->vertices = calloc(1, m->vertexCnt * sizeof(*m->vertices));
	
	for(i = 0; i < m->vertexCnt; i++) {
		vCopy(&obj->faces[i].v, &m->vertices[i].v);
		vCopy(&obj->faces[i].n, &m->vertices[i].n);
		
		if(vMag(&obj->faces[i].n) < 0.1) {
			printf("\n\n----broken normal: %d \n\n", i);
		}
		
		m->vertices[i].t.u = obj->faces[i].t.x * 65535;
		m->vertices[i].t.v = obj->faces[i].t.y * 65535;
	}
	
	// index buffer
	// TODO: fix above code to deduplicate vertices
	m->indexWidth = 2;
	m->indexCnt = m->vertexCnt;
	
	m->indices.w16 = malloc(sizeof(*m->indices.w16) * m->indexCnt);
	CHECK_OOM(m->indices.w16);
	
	for(i = 0; i < m->vertexCnt; i++) m->indices.w16[i] = i;
	
	
	
	printf("-----loaded---------------\n");
	
	m->curFrameIndex = 0;
	VEC_INIT(&m->instances[0]);
	VEC_INIT(&m->instances[1]);
	
	VEC_INIT(&m->instMatrices);
	
	
	return m;
}






RiggedMeshManager* RiggedMeshManager_alloc(GlobalSettings* gs) {
	RiggedMeshManager* mm;
	
	pcalloc(mm);
	
	RiggedMeshManager_init(mm, gs);
	
	return mm;
}


void RiggedMeshManager_init(RiggedMeshManager* mm, GlobalSettings* gs) {
	VEC_INIT(&mm->meshes);
	HT_init(&mm->lookup, 6);
	
	mm->maxInstances = gs->RiggedMeshManager_maxInstances;
	
	mm->mdi = MultiDrawIndirect_alloc(vao_opts, mm->maxInstances, "RiggedMeshManager");
	mm->mdi->isIndexed = 1;
	mm->mdi->indexSize = 2;
	mm->mdi->primMode = GL_TRIANGLES;
	mm->mdi->uniformSetup = (void*)uniformSetup;
	mm->mdi->instanceSetup = (void*)instanceSetup;
	mm->mdi->data = mm;
	

}

void RiggedMeshManager_initGL(RiggedMeshManager* mm, GlobalSettings* gs) {
	MultiDrawIndirect_initGL(mm->mdi);
	
	PCBuffer_startInit(
		&mm->bonesBuffer, 
		mm->maxInstances * sizeof(Matrix), 
		GL_SHADER_STORAGE_BUFFER
	);
	PCBuffer_finishInit(&mm->bonesBuffer);
}

// returns the index of the instance
int RiggedMeshManager_addInstance(RiggedMeshManager* mm, int meshIndex, const RiggedMeshInstance* smi) {
	
	RiggedMesh* msh; 
	RiggedMeshInstance* s;
	
	if(meshIndex >= VEC_LEN(&mm->meshes)) {
		fprintf(stderr, "mesh manager addInstance out of bounds: %d, %d\n", (int)VEC_LEN(&mm->meshes), meshIndex);
		return -1;
	}
	
	mm->totalInstances++;
	
	//printf("adding instance: %d ", meshIndex);
	msh = VEC_DATA(&mm->meshes)[meshIndex];
	VEC_PUSH(&msh->instances[0], *smi);
	VEC_PUSH(&msh->instances[1], *smi);
	VEC_INC(&msh->instMatrices);
	
	//printf("add instance: %d", mm->totalInstances, VEC_LEN(&msh->instances[0]));
	
	return VEC_LEN(&msh->instances[0]);
}

// returns the index of the instance
int RiggedMeshManager_lookupName(RiggedMeshManager* mm, char* name) {
	
	int64_t index;
	
	if(!HT_get(&mm->lookup, name, &index)) {
		printf("Rigged mesh found: %s -> %d\n", name, index);
		return index;
	}
	printf("Rigged mesh not found: %s\n", name);
	return -1;
}

// returns the index if the mesh
int RiggedMeshManager_addMesh(RiggedMeshManager* mm, char* name, RiggedMesh* sm) {
	int index;
	
	
	MDIDrawInfo* di = pcalloc(di);
	
	*di = (MDIDrawInfo){
		.vertices = sm->vertices,
		.vertexCount = sm->vertexCnt,
		
		.indices = sm->indices.w16,
		.indexCount = sm->indexCnt,
	};
	
	MultiDrawIndirect_addMesh(mm->mdi, di);
	
	
	
	VEC_PUSH(&mm->meshes, sm);
	//mm->totalVertices += sm->vertexCnt;
	//mm->totalIndices += sm->indexCnt;
	index = VEC_LEN(&mm->meshes);
	
	HT_set(&mm->lookup, name, index -1);
	
	return index - 1;
}



// should only used for initial setup
void RiggedMeshManager_updateGeometry(RiggedMeshManager* mm) {
	MultiDrawIndirect_updateGeometry(mm->mdi);
}


static void instanceSetup(RiggedMeshManager* mm, RiggedMeshInstShader* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp) {
	
//void RiggedMeshManager_updateMatrices(RiggedMeshManager* mm, PassFrameParams* pfp) {
	int mesh_index, i, j;
	size_t off = 0;
	
	// make sure the matrix buffer has enough space
	if(mm->matBufAlloc < mm->totalInstances) {
		Matrix* p = realloc(mm->matBuf, mm->totalInstances * 2 * sizeof(*mm->matBuf));
		if(!p) {
			printf(stderr, "OOM for matrix buffer in mm\n");
		}
		mm->matBuf = p;
		mm->matBufAlloc = mm->totalInstances * 2;
	}
	
	// update dm matbuf offsets
	for(j = 0; j < diCount; j++) { // j is mesh index
		RiggedMesh* dm = VEC_ITEM(&mm->meshes, j);
		di[j]->numToDraw = 0;
		dm->matBufOffset = off;
		
		off += VEC_LEN(&dm->instances[0]);
	}
	
	/*
	// update dm matbuf offsets
	for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
		RiggedMesh* dm = VEC_ITEM(&mm->meshes, mesh_index);
		
		dm->numToDraw = 0; // clear this out while we're at it	
		dm->matBufOffset = off;
		
		off += VEC_LEN(&dm->instances[0]);
	}
	*/
	
	// walk the components
	ComponentManager* posComp = CES_getCompManager(mm->ces, "position");
	ComponentManager* meshComp = CES_getCompManager(mm->ces, "meshIndex");
	ComponentManager* rotComp = CES_getCompManager(mm->ces, "rotation");
	
	
	int instIndex = 0;
	
	CompManIter cindex, pindex, rindex;
	ComponentManager_start(meshComp, &cindex);
	ComponentManager_start(posComp, &pindex);
	ComponentManager_start(rotComp, &rindex);
	uint32_t eid;
	uint16_t* meshIndex;
	while(meshIndex = ComponentManager_next(meshComp, &cindex, &eid)) {
		Vector* pos;
		//printf("\nseeking in mesh man for %d\n", eid);
		if(!(pos = ComponentManager_nextEnt(posComp, &pindex, eid))) {
		//	 printf("continued\n");
			 continue;
		}
// 		printf("mesh eid %d %d %d\n", eid, cindex, pindex);
		
		RiggedMesh* dm = VEC_ITEM(&mm->meshes, *meshIndex);
		
		
		Matrix m = IDENT_MATRIX, tmp = IDENT_MATRIX;
		
		float d = vDist(pos, &pfp->dp->eyePos);
		
		if(d > 500) continue;
		
		mTransv(pos, &m);
		
		C_Rotation* rot;
	//	printf("\nseeking 2 in mesh man for %d\n", eid);
		if(rot = ComponentManager_nextEnt(rotComp, &rindex, eid)) {
			mRotv(&rot->axis, rot->theta, &m); 
		}
		
		mRot3f(1, 0, 0, F_PI / 2, &m);
		mRot3f(1, 0, 0, dm->defaultRotX, &m);
		mRot3f(0, 1, 0, dm->defaultRotY, &m);
		mRot3f(0, 0, 1, dm->defaultRotZ, &m);
		
		
		mScale3f(dm->defaultScale, dm->defaultScale, dm->defaultScale, &m);
		
		mm->matBuf[dm->matBufOffset + di[*meshIndex]->numToDraw] = m;
		
		di[*meshIndex]->numToDraw++;
	}
	
	
	for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
		RiggedMesh* dm = VEC_ITEM(&mm->meshes, mesh_index);
		
		// TODO make instances switch per frame
		for(i = 0; i < di[mesh_index]->numToDraw; i++) {
			RiggedMeshInstance* dmi = &VEC_ITEM(&dm->instances[0], i);
		
			vmem->m = mm->matBuf[dm->matBufOffset + i];
			vmem->diffuseIndex = dm->texIndex;
			vmem->normalIndex = 0;
			
			vmem++;
		}
	}

	/* 
	pre-component version
	
	for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
		RiggedMesh* dm = VEC_ITEM(&mm->meshes, mesh_index);
		dm->numToDraw = 0;
		
		// TODO make instances switch per frame
		for(i = 0; i < VEC_LEN(&dm->instances[0]); i++) {
			RiggedMeshInstance* dmi = &VEC_ITEM(&dm->instances[0], i);
			Matrix m = IDENT_MATRIX, tmp = IDENT_MATRIX;
			
			float d = vDist(&dmi->pos, &pfp->dp->eyePos);
				
		//	printf("d %f -- ", d);
		//	printf("%f, %f, %f -- ", dmi->pos.x, dmi->pos.y, dmi->pos.z);
		//	printf("%f, %f, %f\n", pfp->dp->eyePos.x, pfp->dp->eyePos.y, pfp->dp->eyePos.z);
			
			if(d > 500) continue;
			dm->numToDraw++;
			
			mTransv(&dmi->pos, &m);
			

			//mTransv(&noise, &m);
			
			// z-up hack for now
			mRot3f(1, 0, 0, F_PI / 2, &m);
			mRot3f(1, 0, 0, dm->defaultRotX, &m);
			mRot3f(0, 1, 0, dm->defaultRotY, &m);
			mRot3f(0, 0, 1, dm->defaultRotZ, &m);
			mScale3f(dm->defaultScale, dm->defaultScale, dm->defaultScale, &m);
			
			//mScalev(&dmi->scale, &m);
			
			// only write sequentially. random access is very bad.
			vmem->m = m;
			vmem->diffuseIndex = dm->texIndex;
			vmem->normalIndex = 0;
			
			vmem++;
		}
		//vmem += VEC_LEN(&dm->instances);
	//	printf("num to draw %d\n", dm->numToDraw);
	}
	*/
	
}




RenderPass* RiggedMeshManager_CreateShadowPass(RiggedMeshManager* m) {
	
	RenderPass* rp;
	PassDrawable* pd;
	
	static ShaderProgram* prog = NULL;
	if(!prog) {
		prog = loadCombinedProgram("riggedMeshInstanced_shadow");
	}

	pd = MultiDrawIndirect_CreateDrawable(m->mdi, prog);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


RenderPass* RiggedMeshManager_CreateRenderPass(RiggedMeshManager* m) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = RiggedMeshManager_CreateDrawable(m);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* RiggedMeshManager_CreateDrawable(RiggedMeshManager* m) {
	return MultiDrawIndirect_CreateDrawable(m->mdi, prog);
}




static void uniformSetup(RiggedMeshManager* mm, GLuint progID) {
	// matrices and uniforms
	GLuint tex_ul;

	glActiveTexture(GL_TEXTURE0 + 8);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mm->tm->tex_id);
	
	tex_ul = glGetUniformLocation(progID, "sTexture");
	glProgramUniform1i(progID, tex_ul, 8);
	
	
		GLunit bonesIndex_ul;
	bonesIndex_ul = glGetProgramResourceIndex(prog->id, GL_SHADER_STORAGE_BLOCK, "boneBuffer");
	glShaderStorageBlockBinding(prog->id, bonesIndex_ul, 0);
	
	PCBuffer_bindActiveRange(mm->bonesBuffer);
	
	
	
	glexit("");
	
	
}

