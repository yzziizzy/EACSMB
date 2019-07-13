
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"


#include "dynamicMesh.h"
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
static void uniformSetup(DynamicMeshManager* dmm, GLuint progID);
static void instanceSetup(DynamicMeshManager* dmm, DynamicMeshInstShader* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp);





// VAO
VAOConfig vao_opts[] = {
	// per vertex
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // position
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // normal
	{0, 2, GL_UNSIGNED_SHORT, 0, GL_TRUE}, // tex
	
	// per instance 
	{1, 1, GL_MATRIX_EXT, 1, GL_FALSE}, // model-world matrix
	{1, 4, GL_UNSIGNED_SHORT, 1, GL_FALSE}, // texture indices: diffuse, normal, metallic, roughness
	
	{0, 0, 0}
};





void initDynamicMeshes() {
	
	PassDrawable* pd;
	

	
//	vao = makeVAO(vao_opts);

	glexit("dynamic mesh vao");
	
	// shader
	prog = loadCombinedProgram("dynamicMeshInstanced");
	
	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("dynamic mesh shader");
	
	//tex = loadBitmapTexture("./assets/textures/gazebo-small.png");
	
	//glActiveTexture(GL_TEXTURE0 + 8);
	//glBindTexture(GL_TEXTURE_2D, tex->tex_id);
	
	glexit("");
	

}




DynamicMesh* DynamicMesh_FromGLTF(gltf_file* gf, int meshIndex) {
	
	DynamicMesh* m = pcalloc(m);
	gltf_mesh* mesh = VEC_ITEM(&gf->meshes, meshIndex);
	
	// TODO: fuse all primitives, convert to GL_TRIANGLES
	
	VEC_EACH(&mesh->primitives, prim_i, prim) {
		if(prim->mode != GL_TRIANGLES) {
			continue;
		}
		
		m->polyMode = GL_TRIANGLES;
		m->vertexCnt = prim->position->count;
		m->vertices = calloc(1, m->vertexCnt * sizeof(*m->vertices));
		
		Vector* positions = malloc(prim->position->bufferView->length);
		gltf_readAccessor(prim->position, positions);
		
		Vector* normals = malloc(prim->normal->bufferView->length);
		gltf_readAccessor(prim->normal, normals);

		uint16_t* texcoords = malloc(prim->texCoord0->bufferView->length);
		gltf_readAccessor(prim->texCoord0, texcoords);
		
		for(int i = 0; i < m->vertexCnt; i++) {
			m->vertices[i].v = positions[i];
			m->vertices[i].n = normals[i];
			m->vertices[i].t.u = texcoords[i * 2]; 
			m->vertices[i].t.v = texcoords[i * 2 + 1];
		}
		
		free(positions);
		free(normals);
		free(texcoords);
	}
	
}

DynamicMesh* DynamicMeshFromOBJ(OBJContents* obj) {
	
	int i;
	DynamicMesh* m;
	
	
	m = calloc(1, sizeof(*m));
	
	m->vertexCnt = 3 * obj->faceCnt;
	m->vertices = calloc(1, m->vertexCnt * sizeof(*m->vertices));
	
	for(i = 0; i < m->vertexCnt; i++) {
		vCopy(&obj->faces[i].v, &m->vertices[i].v);
		vCopy(&obj->faces[i].n, &m->vertices[i].n);
		
		if(vMag(&obj->faces[i].n) < 0.1) {
			//printf("\n\n----broken normal: %d \n\n", i);
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






DynamicMeshManager* dynamicMeshManager_alloc(GlobalSettings* gs) {
	DynamicMeshManager* mm;
	
	pcalloc(mm);
	
	dynamicMeshManager_init(mm, gs);
	
	return mm;
}


void dynamicMeshManager_init(DynamicMeshManager* dmm, GlobalSettings* gs) {
	VEC_INIT(&dmm->meshes);
	HT_init(&dmm->lookup, 6);
	
	dmm->maxInstances = gs->DynamicMeshManager_maxInstances;
	
	dmm->mdi = MultiDrawIndirect_alloc(vao_opts, dmm->maxInstances, "dynamicMeshManager");
	dmm->mdi->isIndexed = 1;
	dmm->mdi->indexSize = 2;
	dmm->mdi->primMode = GL_TRIANGLES;
	dmm->mdi->uniformSetup = (void*)uniformSetup;
	dmm->mdi->instanceSetup = (void*)instanceSetup;
	dmm->mdi->data = dmm;
}

void dynamicMeshManager_initGL(DynamicMeshManager* dmm, GlobalSettings* gs) {
	MultiDrawIndirect_initGL(dmm->mdi);
}

// returns the index of the instance
int dynamicMeshManager_addInstance(DynamicMeshManager* mm, int meshIndex, const DynamicMeshInstance* smi) {
	
	DynamicMesh* msh; 
	DynamicMeshInstance* s;
	
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
int dynamicMeshManager_lookupName(DynamicMeshManager* mm, char* name) {
	
	int64_t index;
	
	if(!HT_get(&mm->lookup, name, &index)) {
		printf("dynamic mesh found: %s -> %d\n", name, index);
		return index;
	}
	printf("dynamic mesh not found: %s\n", name);
	return -1;
}

// returns the index if the mesh
int dynamicMeshManager_addMesh(DynamicMeshManager* mm, char* name, DynamicMesh* sm) {
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
void dynamicMeshManager_updateGeometry(DynamicMeshManager* mm) {
	MultiDrawIndirect_updateGeometry(mm->mdi);
}


static void instanceSetup(DynamicMeshManager* dmm, DynamicMeshInstShader* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp) {
	
//void dynamicMeshManager_updateMatrices(DynamicMeshManager* dmm, PassFrameParams* pfp) {
	int mesh_index, i, j;
	size_t off = 0;
	
	// make sure the matrix buffer has enough space
	if(dmm->matBufAlloc < dmm->totalInstances) {
		Matrix* p = realloc(dmm->matBuf, dmm->totalInstances * 2 * sizeof(*dmm->matBuf));
		if(!p) {
			printf(stderr, "OOM for matrix buffer in dmm\n");
		}
		dmm->matBuf = p;
		dmm->matBufAlloc = dmm->totalInstances * 2;
	}
	
	// update dm matbuf offsets
	for(j = 0; j < diCount; j++) { // j is mesh index
		DynamicMesh* dm = VEC_ITEM(&dmm->meshes, j);
		di[j]->numToDraw = 0;
		dm->matBufOffset = off;
		
		off += VEC_LEN(&dm->instances[0]);
	}
	
	/*
	// update dm matbuf offsets
	for(mesh_index = 0; mesh_index < VEC_LEN(&dmm->meshes); mesh_index++) {
		DynamicMesh* dm = VEC_ITEM(&dmm->meshes, mesh_index);
		
		dm->numToDraw = 0; // clear this out while we're at it	
		dm->matBufOffset = off;
		
		off += VEC_LEN(&dm->instances[0]);
	}
	*/
	
	// walk the components
	ComponentManager* posComp = CES_getCompManager(dmm->ces, "position");
	ComponentManager* meshComp = CES_getCompManager(dmm->ces, "meshIndex");
	ComponentManager* rotComp = CES_getCompManager(dmm->ces, "rotation");
	
	
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
		
		DynamicMesh* dm = VEC_ITEM(&dmm->meshes, *meshIndex);
		
		
		Matrix m = IDENT_MATRIX, tmp = IDENT_MATRIX;
		
		float d = vDist(pos, &pfp->dp->eyePos);
		
// 		if(d > 500) continue;
		
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
		
		dmm->matBuf[dm->matBufOffset + di[*meshIndex]->numToDraw] = m;
		
		di[*meshIndex]->numToDraw++;
	}
	
	
	for(mesh_index = 0; mesh_index < VEC_LEN(&dmm->meshes); mesh_index++) {
		DynamicMesh* dm = VEC_ITEM(&dmm->meshes, mesh_index);
		
		// TODO make instances switch per frame
		for(i = 0; i < di[mesh_index]->numToDraw; i++) {
			DynamicMeshInstance* dmi = &VEC_ITEM(&dm->instances[0], i);
		
			vmem->m = dmm->matBuf[dm->matBufOffset + i];
			vmem->diffuseIndex = dm->texIndex;
			vmem->normalIndex = 0;
			
			vmem++;
		}
	}

	/* 
	pre-component version
	
	for(mesh_index = 0; mesh_index < VEC_LEN(&dmm->meshes); mesh_index++) {
		DynamicMesh* dm = VEC_ITEM(&dmm->meshes, mesh_index);
		dm->numToDraw = 0;
		
		// TODO make instances switch per frame
		for(i = 0; i < VEC_LEN(&dm->instances[0]); i++) {
			DynamicMeshInstance* dmi = &VEC_ITEM(&dm->instances[0], i);
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




RenderPass* DynamicMeshManager_CreateShadowPass(DynamicMeshManager* m) {
	
	RenderPass* rp;
	PassDrawable* pd;
	
	static ShaderProgram* prog = NULL;
	if(!prog) {
		prog = loadCombinedProgram("dynamicMeshInstanced_shadow");
	}

	pd = MultiDrawIndirect_CreateDrawable(m->mdi, prog);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


RenderPass* DynamicMeshManager_CreateRenderPass(DynamicMeshManager* m) {
	
	RenderPass* rp;
	PassDrawable* pd;

	pd = DynamicMeshManager_CreateDrawable(m);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	//rp->fboIndex = LIGHTING;
	
	return rp;
}


PassDrawable* DynamicMeshManager_CreateDrawable(DynamicMeshManager* m) {
	return MultiDrawIndirect_CreateDrawable(m->mdi, prog);
}




static void uniformSetup(DynamicMeshManager* dmm, GLuint progID) {
	// matrices and uniforms
	GLuint tex_ul;

	glActiveTexture(GL_TEXTURE0 + 15);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dmm->tm->tex_id);
	glActiveTexture(GL_TEXTURE0 + 16);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dmm->tmNorm->tex_id);
	glActiveTexture(GL_TEXTURE0 + 17);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dmm->tmMat->tex_id);
	
	tex_ul = glGetUniformLocation(progID, "sTexture");
	glProgramUniform1i(progID, tex_ul, 15);

	tex_ul = glGetUniformLocation(progID, "sNormalTextures");
	glProgramUniform1i(progID, tex_ul, 16);

	tex_ul = glGetUniformLocation(progID, "sMaterialTextures");
	glProgramUniform1i(progID, tex_ul, 17);

	glexit("");
}















static void smm_preFrame(PassFrameParams* pfp, SlowMeshManager* mm);
static void smm_draw(SlowMeshManager* mm, GLuint progID, PassDrawParams* pdp);
static void smm_postFrame(SlowMeshManager* mm);




RenderPass* SlowMeshManager_CreateRenderPass(SlowMeshManager* mm, ShaderProgram* prog) {
	RenderPass* rp;
	PassDrawable* pd;

	pd = SlowMeshManager_CreateDrawable(mm, prog);

	rp = calloc(1, sizeof(*rp));
	RenderPass_init(rp);
	RenderPass_addDrawable(rp, pd);
	
	return rp;
}


PassDrawable* SlowMeshManager_CreateDrawable(SlowMeshManager* mm, ShaderProgram* prog) {
	PassDrawable* pd;

	pd = Pass_allocDrawable("SlowMeshManager");
	pd->data = mm;
	pd->preFrame = smm_preFrame;
	pd->draw = (PassDrawFn)smm_draw;
	pd->postFrame = smm_postFrame;
	pd->prog = prog;
	
	return pd;
}



static void smm_preFrame(PassFrameParams* pfp, SlowMeshManager* mm) {
	int index_offset = 0;
	int vertex_offset = 0;
	int instance_offset = 0;
	int mesh_index;
	void* vmem = PCBuffer_beginWrite(&mm->instVB);
	
	if(!vmem) {
		printf("attempted to update invalid MDI\n");
		return;
	}
/*
	// BUG: bounds checking on pcbuffer inside instanceSetup()
	if(mdi->instanceSetup) {
		(*mdi->instanceSetup)(mdi->data, vmem, VEC_DATA(&mdi->meshes), VEC_LEN(&mdi->meshes), pfp);
	}
	*/
	// BUG: bounds checking on pcbuffer
	
	// set up the indirect draw commands
	if(mm->isIndexed) {
		
		DrawElementsIndirectCommand* cmdsi = PCBuffer_beginWrite(&mm->indirectCmds);
		
		for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
			SlowMeshDrawInfo* di = VEC_ITEM(&mm->meshes, mesh_index);
				
			cmdsi[mesh_index].firstIndex = index_offset; // offset of this mesh into the instances
			cmdsi[mesh_index].count = di->mesh->indexCnt; // number of polys
			
			// offset into instanced vertex attributes
			cmdsi[mesh_index].baseInstance = (mm->maxInstances * ((mm->instVB.nextRegion) % PC_BUFFER_DEPTH)) + instance_offset; 
			// number of instances
			cmdsi[mesh_index].instanceCount = di->numToDraw; 
			cmdsi[mesh_index].baseVertex = vertex_offset;
			
			index_offset += di->mesh->indexCnt;
			vertex_offset += di->mesh->vertexCnt;
			instance_offset += di->numToDraw;
		}
		
	}
	else {
		printf("non-indexed meshes not supported by slow mesh manager.\n");
	}
	
}

static void smm_draw(SlowMeshManager* mm, GLuint progID, PassDrawParams* pdp) {
	size_t cmdOffset;
	
	glBindVertexArray(mm->vao);
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	
	if(mm->isIndexed)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mm->ibo);
	
	PCBuffer_bind(&mm->instVB);
	PCBuffer_bind(&mm->indirectCmds);
	
	cmdOffset = PCBuffer_getOffset(&mm->indirectCmds);
	
	if(mm->isIndexed) {
		glMultiDrawElementsIndirect(mm->primMode, GL_UNSIGNED_SHORT, cmdOffset, VEC_LEN(&mm->meshes), 0);
	}
	else {
		printf("slow mesh manager does not support non-indexed meshes");
	}
	glexit("multidrawarraysindirect");
}



static void smm_postFrame(SlowMeshManager* mm) {
	PCBuffer_afterDraw(&mm->instVB);
	PCBuffer_afterDraw(&mm->indirectCmds);
}









SlowMeshManager* SlowMeshManager_alloc(int maxInstances, int maxMeshes, VAOConfig* vaocfg) {
	SlowMeshManager* mm = pcalloc(mm);
	SlowMeshManager_init(mm, maxInstances, maxMeshes, vaocfg);
	return mm;
}

void SlowMeshManager_init(SlowMeshManager* mm, int maxInstances, int maxMeshes, VAOConfig* vaocfg) {
	VEC_INIT(&mm->meshes);
	mm->maxMeshes = maxMeshes;
	mm->maxInstances = maxInstances;
	mm->vaoConfig = vaocfg;
	mm->isIndexed = 1; // only indexed meshes
	mm->indexSize = 2; // only 16-bit indices
	mm->primMode = GL_TRIANGLES; // only triangles
}

void SlowMeshManager_initGL(SlowMeshManager* mm) {
	
	mm->vao = makeVAO(mm->vaoConfig);
	glBindVertexArray(mm->vao);
	
	mm->vaoGeomStride = calcVAOStride(0, mm->vaoConfig);
	mm->vaoInstStride = calcVAOStride(1, mm->vaoConfig);
	
	PCBuffer_startInit(&mm->instVB, mm->maxInstances * mm->vaoInstStride, GL_ARRAY_BUFFER);
	updateVAO(1, mm->vaoConfig); 
	PCBuffer_finishInit(&mm->instVB);
	
	PCBuffer_startInit(
		&mm->indirectCmds, 
		mm->maxMeshes * sizeof(DrawElementsIndirectCommand), // carefull here
		GL_DRAW_INDIRECT_BUFFER
	);
	PCBuffer_finishInit(&mm->indirectCmds);
}



void SlowMeshManager_RefreshGeometry(SlowMeshManager* mm) {
	int offset;
	
	glBindVertexArray(mm->vao);
	
	// recreate geometry buffer
	if(glIsBuffer(mm->geomVBO)) glDeleteBuffers(1, &mm->geomVBO);
	glGenBuffers(1, &mm->geomVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);

	glBufferStorage(GL_ARRAY_BUFFER, mm->totalVertices * mm->vaoGeomStride, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	// update geometry data
	void* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	offset = 0;
	VEC_EACH(&mm->meshes, i, di) {
		
		memcpy(buf + offset, di->mesh->vertices, di->mesh->vertexCnt * mm->vaoGeomStride);
		offset += di->mesh->vertexCnt * mm->vaoGeomStride;
	}
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
	
	
	// index buffers
	if(mm->isIndexed) {
		
		if(glIsBuffer(mm->ibo)) glDeleteBuffers(1, &mm->ibo);
		glGenBuffers(1, &mm->ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mm->ibo);
		
		glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, mm->totalIndices * mm->indexSize, NULL, GL_MAP_WRITE_BIT);

		uint16_t* ib = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
		
		offset = 0;
		VEC_EACH(&mm->meshes, i, di) {
			memcpy(ib + offset, di->mesh->indices.w8, di->mesh->indexCnt * mm->indexSize);
			offset += di->mesh->indexCnt;
		}
		
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else {
		printf("!!! slowmeshmanager only supports indexed meshes\n");
	}
	
	glexit("");
} 




void SlowMeshManager_AddMesh(SlowMeshManager* mm, DynamicMesh* dm) {
	SlowMeshDrawInfo* di = pcalloc(di);
	
	di->mesh = dm;
	
	VEC_PUSH(&mm->meshes, di);
	mm->totalVertices += dm->vertexCnt;
	mm->totalIndices += dm->indexCnt;
}

// and all instances
void SlowMeshManager_RemoveMesh(SlowMeshManager* mm, DynamicMesh* dm) {
	VEC_EACH(&mm->meshes, i, di) {
		if(di->mesh == dm) {
			free(di);
			mm->totalVertices -= dm->vertexCnt;
			mm->totalIndices -= dm->indexCnt;
			VEC_RM(&mm->meshes, i);
			
			// TODO: remove instances
		}
	}
}


void SlowMeshManager_AddInstance(SlowMeshManager* mm, DynamicMesh* dm, DynamicMeshInstance* inst) {
	VEC_PUSH(&dm->instances[0], *inst);
	mm->totalInstances++;
}

void SlowMeshManager_RemoveInstance(SlowMeshManager* mm, DynamicMesh* dm, DynamicMeshInstance* inst) {
	
	mm->totalInstances--;
}

