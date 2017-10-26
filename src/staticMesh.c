 
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
#include "staticMesh.h"
#include "texture.h"

#include "c_json/json.h"



static GLuint vao;
static GLuint vaoSingle;
static GLuint color_ul, model_ul, view_ul, proj_ul;
static GLuint single_color_ul, single_model_ul, single_view_ul, single_proj_ul;
static ShaderProgram* prog;
static ShaderProgram* progSingle;
Texture* tex;


void initStaticMeshes() {
	
	// VAO
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // normal
		{2, GL_UNSIGNED_SHORT}, // tex
		
		// per instance 
		{4, GL_FLOAT}, // position, scale
		{4, GL_FLOAT}, // direction, rotation
		{4, GL_FLOAT}, // alpha, x, x, x
		
		{0, 0}
	};
	
	vao = makeVAO(opts);
	
	VAOConfig opts_single[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // normal
		{2, GL_UNSIGNED_SHORT}, // tex
		
		{0, 0}
	};
	
	vaoSingle = makeVAO(opts_single);

	glexit("static mesh vao");
	
	// shader
//  	prog = loadCombinedProgram("staticMesh");
	progSingle = loadCombinedProgram("staticMesh");
	prog = loadCombinedProgram("staticMeshInstanced");
	
	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	color_ul = glGetUniformLocation(prog->id, "color");
	
	single_model_ul = glGetUniformLocation(progSingle->id, "mModel");
	single_view_ul = glGetUniformLocation(progSingle->id, "mView");
	single_proj_ul = glGetUniformLocation(progSingle->id, "mProj");
	single_color_ul = glGetUniformLocation(progSingle->id, "color");
	
	glexit("static mesh shader");
	
	tex = loadBitmapTexture("./assets/textures/gazebo-small.png");
	
	glActiveTexture(GL_TEXTURE0 + 7);
	glBindTexture(GL_TEXTURE_2D, tex->tex_id);
	
	glexit("");
}


void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj) {
	
	Matrix model;
	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
//	mScale3f(150, 150, 150, &model);
	//mTrans3f(0,0,0, &model);
	
	glUseProgram(progSingle->id);

	glUniformMatrix4fv(single_model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(single_view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(single_proj_ul, 1, GL_FALSE, &proj->m);
	glUniform3f(single_color_ul, .5, .2, .9);
	
	glBindVertexArray(vaoSingle);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glexit("mesh vbo");
	
	if(m->indexWidth == 0) {
	
		glDrawArrays(GL_TRIANGLES, 0, m->vertexCnt);
		glexit("mesh draw");
		
	}
	else { // indexed rendering
		
		glDrawElements(GL_TRIANGLES, m->indexCnt, GL_UNSIGNED_SHORT, m->indices.w16);
		glexit("");
		
	}
}


void StaticMesh_updateBuffers(StaticMesh* sm) {
	
	glexit("before static mesh vbo load");
	glBindVertexArray(vaoSingle);
	
	glGenBuffers(1, &sm->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sm->vbo);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 24);

	glBufferData(GL_ARRAY_BUFFER, sm->vertexCnt * sizeof(StaticMeshVertex), sm->vertices, GL_STATIC_DRAW);
	glexit("");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("static mesh vbo load");
	
	if(sm->indexWidth > 0) {
		
		// TODO IBO's
	}
	
}



StaticMesh* StaticMeshFromOBJ(OBJContents* obj) {
	
	int i;
	StaticMesh* m;
	
	
	m = calloc(1, sizeof(StaticMesh));
	
	m->vertexCnt = 3 * obj->faceCnt;
	m->vertices = calloc(1, m->vertexCnt * sizeof(StaticMeshVertex));
	
	for(i = 0; i < m->vertexCnt; i++) {
		vCopy(&obj->faces[i].v, &m->vertices[i].v);
		vCopy(&obj->faces[i].n, &m->vertices[i].n);
		
		if(vMag(&obj->faces[i].n) < 0.1) {
			printf("\n\n----broken normal: %d \n\n", i);
		}
		
		m->vertices[i].t.u = obj->faces[i].t.x * 65535;
		m->vertices[i].t.v = obj->faces[i].t.y * 65535;
	}
	
	glexit("before static mesh vbo load");
	glBindVertexArray(vao);
	
	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 24);

	glBufferData(GL_ARRAY_BUFFER, m->vertexCnt * sizeof(StaticMeshVertex), m->vertices, GL_STATIC_DRAW);
	glexit("");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("static mesh vbo load");
	
	
	// index buffer
	// TODO: fix above code to deduplicate vertices
	m->indexWidth = 2;
	m->indexCnt = m->vertexCnt;
	
	m->indices.w16 = malloc(sizeof(*m->indices.w16) * m->indexCnt);
	CHECK_OOM(m->indices.w16);
	
	for(i = 0; i < m->vertexCnt; i++) m->indices.w16[i] = i;
	
	
// 	glGenBuffers(1, &m->ibo);
// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
// 	
// 	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, m->vertexCnt * sizeof(uint16_t), NULL, GL_MAP_WRITE_BIT);
// 
// 	uint16_t* ib = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
// 	for(i = 0; i < m->vertexCnt; i++) ib[i] = i;
// 	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
// 	
// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	
	printf("-----loaded---------------\n");
	VEC_INIT(&m->instances);
	
	return m;
}




MeshManager* meshManager_alloc() {
	MeshManager* mm;
	
	
	mm = calloc(1, sizeof(MeshManager));

	VEC_INIT(&mm->meshes);
	HT_init(&mm->lookup, 6);
	HT_init(&mm->textureLookup, 6);
	
	PCBuffer_startInit(
		&mm->indirectCmds, 
		16 * sizeof(DrawArraysIndirectCommand), 
		GL_DRAW_INDIRECT_BUFFER
	);
	PCBuffer_finishInit(&mm->indirectCmds);
	
	return mm;
}

void meshManager_readConfigFile(MeshManager* mm, char* configPath) {
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
		StaticMesh* sm;
		
		OBJContents obj;
		
		ret = json_obj_get_key(tc, "mesh", &val);
		json_as_string(val, &path);
		
		loadOBJFile(path, 0, &obj);
		sm = StaticMeshFromOBJ(&obj);
		sm->name = strdup(key);
		
		
		ret = json_obj_get_key(tc, "texture", &val);
		if(ret) {
			json_as_string(val, &path);
			
			sm->texIndex = meshManager_addTexture(mm, path);
		}
		
		meshManager_addMesh(mm, sm->name, sm);
		
	}
	
	
}


int meshManager_addTexture(MeshManager* mm, char* path) {
	Texture* tex;
	int64_t index;
	
	// TODO: use lookup first
	
	tex = loadBitmapTexture(path);
	
	index = VEC_LEN(&mm->textures);
	VEC_PUSH(&mm->textures, tex);
	HT_set(&mm->textureLookup, path, index);
	
	return index;
}


// returns the index of the instance
int meshManager_addInstance(MeshManager* mm, int meshIndex, const StaticMeshInstance* smi) {
	
	StaticMesh* msh; 
	StaticMeshInstance* s;
	
	if(meshIndex > VEC_LEN(&mm->meshes)) {
		fprintf(stderr, "mesh manager addInstance out of bounds: %d, %d\n", (int)VEC_LEN(&mm->meshes), meshIndex);
		return -1;
	}
	
	mm->totalInstances++;
	
	msh = VEC_DATA(&mm->meshes)[meshIndex];
	VEC_PUSH(&msh->instances, *smi);
	
	return VEC_LEN(&msh->instances);
}

// returns the index of the instance
int meshManager_lookupName(MeshManager* mm, char* name) {
	
	int64_t index;
	
	if(!HT_get(&mm->lookup, name, &index)) {
		return index;
	}
	
	return -1;
}

// returns the index if the mesh
int meshManager_addMesh(MeshManager* mm, char* name, StaticMesh* sm) {
	int index;
	
	VEC_PUSH(&mm->meshes, sm);
	mm->totalVertices += sm->vertexCnt;
	mm->totalIndices += sm->indexCnt;
	index = VEC_LEN(&mm->meshes);
	
	HT_set(&mm->lookup, name, index -1);
	
	return index;
}



// should only used for initial setup
void meshManager_updateGeometry(MeshManager* mm) {
	
	int i, offset;
	
	glBindVertexArray(vao);
	
	if(glIsBuffer(mm->geomVBO)) glDeleteBuffers(1, &mm->geomVBO);
	glGenBuffers(1, &mm->geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	
	//GL_DYNAMIC_STORAGE_BIT so we can use glBufferSubData() later

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 24);


	glBufferStorage(GL_ARRAY_BUFFER, mm->totalVertices * sizeof(StaticMeshVertex), NULL, GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	offset = 0;
	for(i = 0; i < VEC_LEN(&mm->meshes); i++) {
		
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			offset, 
			VEC_ITEM(&mm->meshes, i)->vertexCnt * sizeof(StaticMeshVertex), 
			VEC_ITEM(&mm->meshes, i)->vertices);
	
		offset += VEC_ITEM(&mm->meshes, i)->vertexCnt * sizeof(StaticMeshVertex);
	}
	
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

void meshManager_updateInstances(MeshManager* mm) {
	
	int i, mesh_index, vertex_offset;
	StaticMeshInstance* buf_ptr;
	
	glBindVertexArray(vao);
	
	// TODO: deal with vbo cycling
	
	
	if(glIsBuffer(mm->instVBO)) glDeleteBuffers(1, &mm->instVBO);
	glGenBuffers(1, &mm->instVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mm->instVBO);
	glexit("");
	//GL_DYNAMIC_STORAGE_BIT so we can use glBufferSubData() later
	glBufferStorage(GL_ARRAY_BUFFER, mm->totalInstances * sizeof(StaticMeshInstance), NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
	glexit("");
	
	
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 3*4*4, 0);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 3*4*4, 1*4*4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 3*4*4, 2*4*4);
	
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	
	buf_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	// copy in data
	vertex_offset = 0;
	for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
		
		int cc = VEC_LEN(&VEC_ITEM(&mm->meshes, mesh_index)->instances); //mm->inst_buf_info[mesh_index].cnt;
		
		memcpy(buf_ptr, VEC_DATA(&VEC_ITEM(&mm->meshes, mesh_index)->instances), cc * sizeof(StaticMeshInstance));
	}
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("");
}




void meshManager_draw(MeshManager* mm, Matrix* view, Matrix* proj) {
	
	GLuint tex_ul;
	Matrix model;
	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
	// HACK fix later
	mScale3f(150, 150, 150, &model);
	//mTrans3f(0,0,0, &model);
	
	tex_ul = glGetUniformLocation(prog->id, "sTexture");
	glProgramUniform1i(prog->id, tex_ul, 7);
	
	glUseProgram(prog->id);
	glexit("");

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj->m);
	glUniform3f(color_ul, .5, .2, .9);
	
	
	DrawArraysIndirectCommand* cmds = PCBuffer_beginWrite(&mm->indirectCmds);
	
	int index_offset = 0;
	int instance_offset = 0;
	int mesh_index;
	for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
		StaticMesh* sm = VEC_ITEM(&mm->meshes, mesh_index);
			
		cmds[mesh_index].first = index_offset; // offset of this mesh into the instances
		cmds[mesh_index].count = sm->vertexCnt; // number of polys
		
		// offset into instanced vertex attributes
		cmds[mesh_index].baseInstance = instance_offset; 
		// number of instances
		cmds[mesh_index].instanceCount = VEC_LEN(&sm->instances); 
		
		index_offset += sm->vertexCnt;// * sizeof(DynamicMeshVertex);//sm->indexCnt;
		instance_offset += VEC_LEN(&sm->instances);
		
	}
	
	PCBuffer_bind(&mm->indirectCmds);
	
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mm->instVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mm->geomIBO);
	
	
	glMultiDrawArraysIndirect(GL_TRIANGLES, 0, VEC_LEN(&mm->meshes), 0);
	glexit("multidrawarraysindirect");
	
	PCBuffer_afterDraw(&mm->indirectCmds);

}


