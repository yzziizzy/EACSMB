
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

// old?
static void core_draw(DynamicMeshManager* dmm);
static void preFrame(PassFrameParams* pfp, DynamicMeshManager* dmm);
static void draw(DynamicMeshManager* dmm, GLuint progID, PassDrawParams* pdp);
static void postFrame(DynamicMeshManager* dmm);


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
	{1, 2, GL_UNSIGNED_SHORT, 1, GL_FALSE}, // texture indices
	
	{0, 0, 0}
};





void initDynamicMeshes() {
	
	PassDrawable* pd;
	

	
	vao = makeVAO(vao_opts);

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
	
	
	/*
	pd = Pass_allocDrawable("dynamic mesh manager");
	pd->preFrame = preFrame;
	pd->draw = draw;
	pd->postFrame = postFrame;
	pd->prog = prog;
	
	Pass_registerDrawable(pd);
	*/
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






DynamicMeshManager* dynamicMeshManager_alloc(int maxInstances) {
	DynamicMeshManager* mm;
	GLbitfield flags;
	size_t vbo_size;
	
	
	mm = calloc(1, sizeof(*mm));

	VEC_INIT(&mm->meshes);
	HT_init(&mm->lookup, 6);
	//HT_init(&mm->textureLookup, 6);
	
	mm->maxInstances = maxInstances;
	
	
	mm->mdi = MultiDrawIndirect_alloc(vao_opts, maxInstances);
	mm->mdi->isIndexed = 1;
	mm->mdi->indexSize = 2;
	mm->mdi->primMode = GL_TRIANGLES;
	mm->mdi->uniformSetup = (void*)uniformSetup;
	mm->mdi->instanceSetup = (void*)instanceSetup;
	mm->mdi->data = mm;
	
	
	
	glBindVertexArray(vao);
	
	PCBuffer_startInit(&mm->instVB, mm->maxInstances * sizeof(Matrix), GL_ARRAY_BUFFER);
	updateVAO(1, vao_opts); 
	PCBuffer_finishInit(&mm->instVB);
	
	/*
	PCBuffer_startInit(
		&mm->indirectCmds, 
		16 * sizeof(DrawArraysIndirectCommand), 
		GL_DRAW_INDIRECT_BUFFER
	);
	PCBuffer_finishInit(&mm->indirectCmds);
	*/
	
	
	return mm;
}

void dynamicMeshManager_readConfigFile(DynamicMeshManager* mm, char* configPath) {
	int ret;
	struct json_obj* o;
	void* iter;
	char* key, *texName, *tmp;
	struct json_value* v, *tc;
	json_file_t* jsf;
	
	
	jsf = json_load_path(configPath);
	
	json_value_t* tex;
	json_obj_get_key(jsf->root, "textures", &tex);
	
	
	iter = NULL;
	
	while(json_obj_next(jsf->root, &iter, &key, &tc)) {
		json_value_t* val;
		char* path;
		DynamicMesh* dm;
		
		OBJContents obj;
		
		ret = json_obj_get_key(tc, "mesh", &val);
		json_as_string(val, &path);
		
		loadOBJFile(path, 0, &obj);
		dm = DynamicMeshFromOBJ(&obj);
		dm->name = strdup(key);
		
		
		ret = json_obj_get_key(tc, "texture", &val);
		if(!ret) {
			json_as_string(val, &path);
			
			dm->texIndex = TextureManager_reservePath(mm->tm, path);
			printf("dmm: %d %s\n", dm->texIndex, path);
		}

#define grab_json_val(str, field, def) \
		dm->field = def; \
		if(!json_obj_get_key(tc, str, &val)) { \
			json_as_float(val, &dm->field); \
		}

		grab_json_val("scale", defaultScale, 1.0)
		grab_json_val("rotDegX", defaultRotX, 0.0)
		grab_json_val("rotDegY", defaultRotY, 0.0)
		grab_json_val("rotDegZ", defaultRotZ, 0.0)
		
		// radians are not easy to edit in a config file, so it's in degrees
		dm->defaultRotX *= F_PI / 180.0;  
		dm->defaultRotY *= F_PI / 180.0;  
		dm->defaultRotZ *= F_PI / 180.0;  
		

		int ind = dynamicMeshManager_addMesh(mm, dm->name, dm);
		printf("DM added mesh %d: %s \n", ind, dm->name);
		
	}
	
	
}

/*
int dynamicMeshManager_addTexture(DynamicMeshManager* mm, char* path) {
	Texture* tex;
	int64_t index;
	
	// TODO: use lookup first
	
	tex = loadBitmapTexture(path);
	
	index = VEC_LEN(&mm->textures);
	VEC_PUSH(&mm->textures, tex);
	HT_set(&mm->textureLookup, path, index);
	
	return index;
}
*/

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
	mm->totalVertices += sm->vertexCnt;
	mm->totalIndices += sm->indexCnt;
	index = VEC_LEN(&mm->meshes);
	
	HT_set(&mm->lookup, name, index -1);
	
	return index - 1;
}



// should only used for initial setup
void dynamicMeshManager_updateGeometry(DynamicMeshManager* mm) {
	
	int i;
	size_t offset;
	
	
	MultiDrawIndirect_updateGeometry(mm->mdi);
	
	/*
	glBindVertexArray(vao);
	
	if(glIsBuffer(mm->geomVBO)) glDeleteBuffers(1, &mm->geomVBO);
	glGenBuffers(1, &mm->geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	
	updateVAO(0, vao_opts); 

	glBufferStorage(GL_ARRAY_BUFFER, mm->totalVertices * sizeof(DynamicMeshVertex), NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	void* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	offset = 0;
	for(i = 0; i < VEC_LEN(&mm->meshes); i++) {
		DynamicMesh* dm = VEC_ITEM(&mm->meshes, i);
		
		memcpy(buf + offset, dm->vertices, dm->vertexCnt * sizeof(DynamicMeshVertex));
		
		offset += dm->vertexCnt * sizeof(DynamicMeshVertex);
	}
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	
	
	
	// index buffers
	if(glIsBuffer(mm->geomIBO)) glDeleteBuffers(1, &mm->geomIBO);
	glGenBuffers(1, &mm->geomIBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mm->geomIBO);
	
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, mm->totalIndices * sizeof(uint16_t), NULL, GL_MAP_WRITE_BIT);

	uint16_t* ib = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	offset = 0;
	for(i = 0; i < VEC_LEN(&mm->meshes); i++) {
		
		memcpy(ib + offset, VEC_ITEM(&mm->meshes, i)->indices.w16, VEC_ITEM(&mm->meshes, i)->indexCnt * sizeof(uint16_t));
		//for(i = 0; i < m->vertexCnt; i++) ib[i] = i;
		
		offset += VEC_ITEM(&mm->meshes, i)->indexCnt;
	}
	
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
	*/
	
	
	glexit(__FILE__);
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
		di[j]->numToDraw = 4;
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
	
	int cindex = -1;
	int pindex = -1;
	int rindex = -1;
	uint32_t eid;
	uint16_t* meshIndex;
	while(meshIndex = ComponentManager_next(meshComp, &cindex, &eid)) {
		//printf("eid %d %d %d\n", eid, cindex, pindex);
		Vector* pos;
		if(!(pos = ComponentManager_nextEnt(posComp, &pindex, eid))) {
			 printf("continued\n");
			 continue;
		}
		printf("%d - %f,%f,%f\n", *meshIndex, pos->x, pos->y, pos->z);
		
		DynamicMesh* dm = VEC_ITEM(&dmm->meshes, *meshIndex);
		
		
		Matrix m = IDENT_MATRIX, tmp = IDENT_MATRIX;
		
		float d = vDist(pos, &pfp->dp->eyePos);
		
		if(d > 500) continue;
		
		mTransv(pos, &m);
	
		C_Rotation* rot;
		if(rot = ComponentManager_nextEnt(rotComp, &rindex, eid)) {
			mRotv(&rot->axis, rot->theta, &m); 
		}
		
		mRot3f(1, 0, 0, F_PI / 2, &m);
		mRot3f(1, 0, 0, dm->defaultRotX, &m);
		mRot3f(0, 1, 0, dm->defaultRotY, &m);
		mRot3f(0, 0, 1, dm->defaultRotZ, &m);
		
	
		mScale3f(dm->defaultScale, dm->defaultScale, dm->defaultScale, &m);			

			
		dmm->matBuf[dm->matBufOffset + dm->numToDraw] = m;
		di[*meshIndex]->numToDraw++;
	}
	
	
	
	
	
	
	
	//DynamicMeshInstShader* vmem = PCBuffer_beginWrite(&dmm->instVB);
	
	//if(!vmem) {
		//printf("attempted to update invalid dynamic mesh manager\n");
		//return;
	//}



	for(mesh_index = 0; mesh_index < VEC_LEN(&dmm->meshes); mesh_index++) {
		DynamicMesh* dm = VEC_ITEM(&dmm->meshes, mesh_index);
		
		// TODO make instances switch per frame
		for(i = 0; i < dm->numToDraw; i++) {
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




static void preFrame(PassFrameParams* pfp, DynamicMeshManager* mm) {
	
	// pre-mdi
//	dynamicMeshManager_updateMatrices(mm, pfp);
	
	/* now inside mdi
	
	// set up the indirect draw commands
	DrawArraysIndirectCommand* cmds = PCBuffer_beginWrite(&mm->indirectCmds);
	
	int index_offset = 0;
	int instance_offset = 0;
	int mesh_index;
	for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
		DynamicMesh* dm = VEC_ITEM(&mm->meshes, mesh_index);
			
		cmds[mesh_index].first = index_offset; // offset of this mesh into the instances
		cmds[mesh_index].count = dm->indexCnt; // number of polys
		
		// offset into instanced vertex attributes
		cmds[mesh_index].baseInstance = (mm->maxInstances * ((mm->instVB.nextRegion) % PC_BUFFER_DEPTH)) + instance_offset; 
		// number of instances
		cmds[mesh_index].instanceCount = dm->numToDraw; //VEC_LEN(&dm->instances[0]); 
	//printf("instances %d %d %d %d  \n", mesh_index, dm->indexCnt, VEC_LEN(&dm->instances[0]), instance_offset );
		
		index_offset += dm->indexCnt;// * sizeof(DynamicMeshVertex);//dm->indexCnt;
		instance_offset += dm->numToDraw; //VEC_LEN(&dm->instances[0]);
	}
	
	*/
}

// this one has to handle different views, such as shadow mapping and reflections

static void draw(DynamicMeshManager* dmm, GLuint progID, PassDrawParams* pdp) {
		
	// matrices and uniforms
	GLuint tex_ul;

	glActiveTexture(GL_TEXTURE0 + 8);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dmm->tm->tex_id);
	
	tex_ul = glGetUniformLocation(progID, "sTexture");
	glProgramUniform1i(progID, tex_ul, 8);
	glexit("");

	// ---------------------------------
	
	core_draw(dmm);

}

static void core_draw(DynamicMeshManager* dmm) {
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, dmm->geomVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dmm->geomIBO);
	
	PCBuffer_bind(&dmm->instVB);
	PCBuffer_bind(&dmm->indirectCmds);
	
	glMultiDrawArraysIndirect(GL_TRIANGLES, 0, VEC_LEN(&dmm->meshes), 0);
	glexit("multidrawarraysindirect");
}


static void postFrame(DynamicMeshManager* mm) {
	PCBuffer_afterDraw(&mm->instVB);
	PCBuffer_afterDraw(&mm->indirectCmds);
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
	PassDrawable* pd;

	//pd = Pass_allocDrawable("DynamicMeshManager");
	//pd->data = m;
	//pd->preFrame = preFrame;
	//pd->draw = (PassDrawFn)draw;
	//pd->postFrame = postFrame;
	//pd->prog = prog;
	
	pd = MultiDrawIndirect_CreateDrawable(m->mdi, prog);
	
	return pd;
}




static void uniformSetup(DynamicMeshManager* dmm, GLuint progID) {
	// matrices and uniforms
	GLuint tex_ul;

	glActiveTexture(GL_TEXTURE0 + 8);
	glBindTexture(GL_TEXTURE_2D_ARRAY, dmm->tm->tex_id);
	
	tex_ul = glGetUniformLocation(progID, "sTexture");
	glProgramUniform1i(progID, tex_ul, 8);
	glexit("");
}

//static void instanceSetup(DynamicMeshManager* dmm, TerrainPatchInstance* vmem, MDIDrawInfo** di, int diCount, PassFrameParams* pfp) {
	
	/* example code from map.c
	int i, j;
	int x, y;
	
	for(j = 0; j < diCount; j++) {
		di[j]->numToDraw = 4;
			
		for(i = 0; i < 1; i++) { // each instance
			vmem[i].offx = i * 256; // multiplier should be mb->w
			vmem[i].offy = i * 256;
		}
		
		di++;
	}
	*/
	
//	dynamicMeshManager_updateMatrices(mm, pfp);
//}
