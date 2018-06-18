
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "common_gl.h"
#include "common_math.h"


#include "dynamicMesh.h"

#include "utilities.h"
#include "objloader.h"
#include "shader.h"
#include "pass.h"

#include "c_json/json.h"






static GLuint vao;
static GLuint color_ul, model_ul, view_ul, proj_ul;
static ShaderProgram* prog;
Texture* tex;


static void preFrame(PassFrameParams* pfp, DynamicMeshManager* dmm);
static void draw(PassDrawParams* pdp, GLuint progID, DynamicMeshManager* dmm);
static void postFrame(DynamicMeshManager* dmm);



void initDynamicMeshes() {
	
	PassDrawable* pd;
	
	// VAO
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // normal
		{2, GL_UNSIGNED_SHORT}, // tex
		
		// per instance 
		{4, GL_FLOAT}, // model-world matrix
		{4, GL_FLOAT}, // 
		{4, GL_FLOAT}, // 
		{4, GL_FLOAT}, // 
		
		{0, 0}
	};
	
	vao = makeVAO(opts);

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
	
	glBindVertexArray(vao);
	
	
	PCBuffer_startInit(&mm->instVB, mm->maxInstances * sizeof(Matrix), GL_ARRAY_BUFFER);

	// position matrix 	
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4*4*4 + 2*2, 0);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4*4*4 + 2*2, 1*4*4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4*4*4 + 2*2, 2*4*4);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4*4*4 + 2*2, 3*4*4);
	
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	
	// texture indices
	glEnableVertexAttribArray(7);
	glVertexAttribIPointer(7, 2, GL_UNSIGNED_SHORT, 4*4*4 + 2*2, 4*4*4);
	glVertexAttribDivisor(7, 1);
	
	
	PCBuffer_finishInit(&mm->instVB);
	
	
	PCBuffer_startInit(
		&mm->indirectCmds, 
		16 * sizeof(DrawArraysIndirectCommand), 
		GL_DRAW_INDIRECT_BUFFER
	);
	PCBuffer_finishInit(&mm->indirectCmds);

	
	
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
	
	glBindVertexArray(vao);
	
	if(glIsBuffer(mm->geomVBO)) glDeleteBuffers(1, &mm->geomVBO);
	glGenBuffers(1, &mm->geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 24);

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
	
	
	
	
	
	glexit(__FILE__);
}



void dynamicMeshManager_updateMatrices(DynamicMeshManager* dmm, PassFrameParams* pfp) {
	int mesh_index, i;
	
	
	
	DynamicMeshInstShader* vmem = PCBuffer_beginWrite(&dmm->instVB);
	
	if(!vmem) {
		printf("attempted to update invalid dynamic mesh manager\n");
		return;
	}

	
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
			
			// only write sequentially. random access is very bad.
			*vmem = di;
			vmem++;
		}
		//vmem += VEC_LEN(&dm->instances);
	//	printf("num to draw %d\n", dm->numToDraw);
	}
	
}





void dynamicMeshManager_draw(DynamicMeshManager* mm, PassFrameParams* pfp) {
	
	GLuint tex_ul;
	Matrix model;
	
	
	
	dynamicMeshManager_updateMatrices(mm, pfp);
	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
	// HACK fix later
	mScale3f(150, 150, 150, &model);
	//mTrans3f(0,0,0, &model);
	
	// TODO: move to intermediate initialization stage
	glActiveTexture(GL_TEXTURE0 + 8);
	glBindTexture(GL_TEXTURE_2D_ARRAY, mm->tm->tex_id);
	
	tex_ul = glGetUniformLocation(prog->id, "sTexture");
	glProgramUniform1i(prog->id, tex_ul, 8);
	
	glUseProgram(prog->id);
	glexit("");

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &pfp->dp->mWorldView->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &pfp->dp->mViewProj->m);
	glUniform3f(color_ul, .5, .2, .9);
	
	

	//printf("instance count %d, %d\n", cmds[0].instanceCount, cmds[0].count);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mm->geomIBO);
	
	
	PCBuffer_bind(&mm->instVB);
	
	
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);

	
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
		instance_offset += VEC_LEN(&dm->instances[0]);
	}
	
	PCBuffer_bind(&mm->indirectCmds);

 	glMultiDrawArraysIndirect(GL_TRIANGLES, 0, VEC_LEN(&mm->meshes), 0);
	glexit("multidrawarraysindirect");
	
	PCBuffer_afterDraw(&mm->instVB);
	PCBuffer_afterDraw(&mm->indirectCmds);
	
}




static void preFrame(PassFrameParams* pfp, DynamicMeshManager* mm) {
	
	dynamicMeshManager_updateMatrices(mm, pfp);
	
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
		cmds[mesh_index].instanceCount = VEC_LEN(&dm->instances[0]); 
		
		index_offset += dm->indexCnt;// * sizeof(DynamicMeshVertex);//dm->indexCnt;
		instance_offset += VEC_LEN(&dm->instances[0]);
		
	}
	
	
	

	
	
}

// this one has to handle different views, such as shadow mapping and reflections

static void draw(PassDrawParams* pdp, GLuint progID, DynamicMeshManager* dmm) {
	
	
		// matrices and uniforms
	GLuint tex_ul;
	Matrix model;
	

	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
	// HACK fix later
	mScale3f(150, 150, 150, &model);
	//mTrans3f(0,0,0, &model);
	
	tex_ul = glGetUniformLocation(prog->id, "sTexture");
	glProgramUniform1i(prog->id, tex_ul, 8);
	
	glUseProgram(prog->id);
	glexit("");

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, model.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, pdp->mWorldView->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, pdp->mViewProj->m);
	glUniform3f(color_ul, .5, .2, .9);
	
	// ---------------------------------
	
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


