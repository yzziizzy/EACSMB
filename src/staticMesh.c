 
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
static GLuint color_ul, model_ul, view_ul, proj_ul;
static ShaderProgram* prog;
Texture* tex;


void initStaticMeshes() {
	
	// VAO
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // normal
		{2, GL_UNSIGNED_SHORT}, // tex
		
		// per instance 
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // direction
		{3, GL_FLOAT}, // scale
		
		{0, 0}
	};
	
	vao = makeVAO(opts);

	glexit("static mesh vao");
	
	// shader
//  	prog = loadCombinedProgram("staticMesh");
	prog = loadCombinedProgram("staticMeshInstanced");
	
	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("static mesh shader");
	
	tex = loadBitmapTexture("./assets/textures/colornoise.png");
	
	glActiveTexture(GL_TEXTURE0 + 7);
	glBindTexture(GL_TEXTURE_2D, tex->tex_id);
	
	glexit("");
}


void drawStaticMesh(StaticMesh* m, Matrix* view, Matrix* proj) {
	
	Matrix model;
	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
	mScale3f(150, 150, 150, &model);
	//mTrans3f(0,0,0, &model);
	
	glUseProgram(prog->id);

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj->m);
	glUniform3f(color_ul, .5, .2, .9);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glexit("mesh vbo");
	

	
	glDrawArrays(GL_TRIANGLES, 0, m->vertexCnt);
	glexit("mesh draw");
	
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
	
	printf("-----loaded---------------\n");
	VEC_INIT(&m->instances);
	
	return m;
}




MeshManager* meshManager_alloc() {
	MeshManager* mm;
	
	
	mm = calloc(1, sizeof(MeshManager));

	VEC_INIT(&mm->meshes);
//	VEC_INIT(&mm->instances);
	
//	glBindVertexArray(vao);
	
	
	//local vbo for collected mesh geometry
	//glGenBuffers(2, mm->instVBO);

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
		
		meshManager_addMesh(mm, sm);
		
	}
	
	
}


int meshManager_addTexture(MeshManager* mm, char* path) {
	Texture* tex;
	int index;
	
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

// returns the index if the mesh
int meshManager_addMesh(MeshManager* mm, StaticMesh* sm) {
	
	VEC_PUSH(&mm->meshes, sm);
	mm->totalVertices += sm->vertexCnt;
	
	
	return VEC_LEN(&mm->meshes);
}



// should only used for initial setup
void meshManager_updateGeometry(MeshManager* mm) {
	
	int i, offset;
	
	glBindVertexArray(vao);
	
	if(glIsBuffer(mm->geomVBO)) glDeleteBuffers(1, &mm->geomVBO);
	glGenBuffers(1, &mm->geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	
	//GL_DYNAMIC_STORAGE_BIT so we can use glBufferSubData() later
	
// 	glEnableVertexAttribArray(0);
// 	glEnableVertexAttribArray(1);
// 	glEnableVertexAttribArray(2);
// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*3*4 + 4, 0);
// 	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*3*4 + 4, 1*3*4);
// 	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 2*3*4);
/*
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 24);
*/


	glBufferStorage(GL_ARRAY_BUFFER, mm->totalVertices * sizeof(StaticMeshVertex), NULL, GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	offset = 0;
	for(i = 0; i < VEC_LEN(&mm->meshes); i++) {
		
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			offset, 
			VEC_ITEM(&mm->meshes, i)->vertexCnt * sizeof(StaticMeshVertex), 
			VEC_ITEM(&mm->meshes, i)->vertices);
		//glBufferData(GL_ARRAY_BUFFER, m->vertexCnt * sizeof(StaticMeshVertex), m->vertices, GL_STATIC_DRAW);
	
		offset += VEC_ITEM(&mm->meshes, i)->vertexCnt * sizeof(StaticMeshVertex);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
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
	printf("total instances %d\n", mm->totalInstances);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3*3*4, 0);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3*3*4, 1*3*4);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3*3*4, 2*3*4);
	
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	
	buf_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	// copy in data
	vertex_offset = 0;
	for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
		// TODO: offsets for rendering
		
		int cc = VEC_LEN(&VEC_ITEM(&mm->meshes, mesh_index)->instances); //mm->inst_buf_info[mesh_index].cnt;
		
		memcpy(buf_ptr, VEC_DATA(&VEC_ITEM(&mm->meshes, mesh_index)->instances), cc * sizeof(StaticMeshInstance));
	}
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("");
}



typedef  struct {
	GLuint  count;
	GLuint  instanceCount;
	GLuint  first;
	GLuint  baseInstance;
} DrawArraysIndirectCommand;


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
	
	
	DrawArraysIndirectCommand cmds[20];
	
	cmds[0].count = VEC_ITEM(&mm->meshes, 0)->vertexCnt; // number of polys
	cmds[0].instanceCount = VEC_LEN(&VEC_ITEM(&mm->meshes, 0)->instances); // number of instances
	cmds[0].first = 0; // offset of this mesh into the instances
	cmds[0].baseInstance = 0; // offset into instanced vertex attributes

	//printf("instance count %d, %d\n", cmds[0].instanceCount, cmds[0].count);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mm->instVBO);
	
//	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	

	glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, cmds[0].count, cmds[0].instanceCount, 0);
//	glMultiDrawArraysIndirect(GL_TRIANGLES, cmds, 1, 0);
	glexit("multidrawarraysindirect");
	
	
}


