
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

#include "c_json/json.h"



static GLuint vao;
static GLuint color_ul, model_ul, view_ul, proj_ul;
static ShaderProgram* prog;
Texture* tex;






void initDynamicMeshes() {
	
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
		{4, GL_FLOAT}, // alpha, x, x, x
		
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
	
	tex = loadBitmapTexture("./assets/textures/gazebo-small.png");
	
	glActiveTexture(GL_TEXTURE0 + 8);
	glBindTexture(GL_TEXTURE_2D, tex->tex_id);
	
	glexit("");
}



/*
void DynamicMesh_updateBuffers(DynamicMesh* sm) {
	
	glexit("before dynamic mesh vbo load");
	glBindVertexArray(vaoSingle);
	
	glGenBuffers(1, &sm->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, sm->vbo);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 24);

	glBufferData(GL_ARRAY_BUFFER, sm->vertexCnt * sizeof(DynamicMeshVertex), sm->vertices, GL_STATIC_DRAW);
	glexit("");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("dynamic mesh vbo load");
	
	if(sm->indexWidth > 0) {
		
		// TODO IBO's
	}
	
}*/



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
	
	glexit("before dynamic mesh vbo load");
	glBindVertexArray(vao);
	glexit("");
	
	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2*3*4 + 4, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 24);

	glBufferData(GL_ARRAY_BUFFER, m->vertexCnt * sizeof(DynamicMeshVertex), m->vertices, GL_STATIC_DRAW);
	glexit("");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("dynamic mesh vbo load");
	
	printf("-----loaded---------------\n");
	VEC_INIT(&m->instances);
	VEC_INIT(&m->instMatrices);
	
	return m;
}




DynamicMeshManager* dynamicMeshManager_alloc() {
	DynamicMeshManager* mm;
	
	
	mm = calloc(1, sizeof(*mm));

	VEC_INIT(&mm->meshes);
	HT_init(&mm->lookup, 6);
	HT_init(&mm->textureLookup, 6);
//	VEC_INIT(&mm->instances);
	
//	glBindVertexArray(vao);
	
	
	//local vbo for collected mesh geometry
	//glGenBuffers(2, mm->instVBO);
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
		if(ret) {
			json_as_string(val, &path);
			
			dm->texIndex = dynamicMeshManager_addTexture(mm, path);
		}
		
		dynamicMeshManager_addMesh(mm, dm->name, dm);
		
	}
	
	
}


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


// returns the index of the instance
int dynamicMeshManager_addInstance(DynamicMeshManager* mm, int meshIndex, const DynamicMeshInstance* smi) {
	
	DynamicMesh* msh; 
	DynamicMeshInstance* s;
	
	if(meshIndex > VEC_LEN(&mm->meshes)) {
		fprintf(stderr, "mesh manager addInstance out of bounds: %d, %d\n", (int)VEC_LEN(&mm->meshes), meshIndex);
		return -1;
	}
	
	mm->totalInstances++;
	
	msh = VEC_DATA(&mm->meshes)[meshIndex];
	VEC_PUSH(&msh->instances, *smi);
	VEC_INC(&msh->instMatrices);
	printf("mat len %d \n",  VEC_LEN(&msh->instMatrices));
	
	return VEC_LEN(&msh->instances);
}

// returns the index of the instance
int dynamicMeshManager_lookupName(DynamicMeshManager* mm, char* name) {
	
	int64_t index;
	
	if(!HT_get(&mm->lookup, name, &index)) {
		return index;
	}
	
	return -1;
}

// returns the index if the mesh
int dynamicMeshManager_addMesh(DynamicMeshManager* mm, char* name, DynamicMesh* sm) {
	int index;
	
	VEC_PUSH(&mm->meshes, sm);
	mm->totalVertices += sm->vertexCnt;
	index = VEC_LEN(&mm->meshes);
	
	HT_set(&mm->lookup, name, index -1);
	
	return index;
}



// should only used for initial setup
void dynamicMeshManager_updateGeometry(DynamicMeshManager* mm) {
	
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


	glBufferStorage(GL_ARRAY_BUFFER, mm->totalVertices * sizeof(DynamicMeshVertex), NULL, GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	offset = 0;
	for(i = 0; i < VEC_LEN(&mm->meshes); i++) {
		
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			offset, 
			VEC_ITEM(&mm->meshes, i)->vertexCnt * sizeof(DynamicMeshVertex), 
			VEC_ITEM(&mm->meshes, i)->vertices);
		//glBufferData(GL_ARRAY_BUFFER, m->vertexCnt * sizeof(DynamicMeshVertex), m->vertices, GL_STATIC_DRAW);
	
		offset += VEC_ITEM(&mm->meshes, i)->vertexCnt * sizeof(DynamicMeshVertex);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	glexit(__FILE__);
}

void dynamicMeshManager_updateInstances(DynamicMeshManager* mm) {
	
	int i, mesh_index, vertex_offset;
	DynamicMeshInstance* buf_ptr;
	
	printf("updateing dynamic mesh instances\n");
	
	glBindVertexArray(vao);
	
	// TODO: deal with vbo cycling
	
	// HACK
	dynamicMeshManager_updateMatrices(mm);
	
	if(glIsBuffer(mm->instVBO)) glDeleteBuffers(1, &mm->instVBO);
	glGenBuffers(1, &mm->instVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mm->instVBO);
	glexit("");
	//GL_DYNAMIC_STORAGE_BIT so we can use glBufferSubData() later
	glBufferStorage(GL_ARRAY_BUFFER, mm->totalInstances * sizeof(Matrix), NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT);
	glexit("");
	printf("total instances %d\n", mm->totalInstances);
	
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4*4*4, 0);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4*4*4, 1*4*4);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4*4*4, 2*4*4);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4*4*4, 3*4*4);
	
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	
	buf_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	// copy in data
	vertex_offset = 0;
	for(mesh_index = 0; mesh_index < VEC_LEN(&mm->meshes); mesh_index++) {
		// TODO: offsets for rendering
		
		int instlen = VEC_LEN(&VEC_ITEM(&mm->meshes, mesh_index)->instMatrices); //mm->inst_buf_info[mesh_index].cnt;
		
		memcpy(
			buf_ptr, 
			VEC_DATA(&VEC_ITEM(&mm->meshes, mesh_index)->instMatrices), 
			instlen * sizeof(Matrix)
		);
	}
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("");
}



void dynamicMeshManager_updateMatrices(DynamicMeshManager* dmm) {
	int mesh_index, i;
	
	for(mesh_index = 0; mesh_index < VEC_LEN(&dmm->meshes); mesh_index++) {
		DynamicMesh* dm = VEC_ITEM(&dmm->meshes, mesh_index);
		
		for(i = 0; i < VEC_LEN(&dm->instances); i++) {
			DynamicMeshInstance* dmi = &VEC_ITEM(&dm->instances, i);
			Matrix m = IDENT_MATRIX, tmp = IDENT_MATRIX;
			
			mTransv(&dmi->pos, &m);
			
			// z-up hack for now
			mRot3f(1, 0, 0, F_PI / 2, &m);
			//mScalev(&dmi->scale, &m);
			
			VEC_ITEM(&dm->instMatrices, i) = m;
		}
		
	}
}




typedef  struct {
	GLuint  count;
	GLuint  instanceCount;
	GLuint  first;
	GLuint  baseInstance;
} DrawArraysIndirectCommand;


void dynamicMeshManager_draw(DynamicMeshManager* mm, Matrix* view, Matrix* proj) {
	
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





