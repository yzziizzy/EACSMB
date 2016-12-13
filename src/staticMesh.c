 
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



static GLuint vao;
static GLuint color_ul, model_ul, view_ul, proj_ul;
static ShaderProgram* prog;

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
	prog = loadCombinedProgram("staticMesh");
	
	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("static mesh shader");
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
	m->vertices = malloc(m->vertexCnt * sizeof(StaticMeshVertex));
	
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

	//glBufferData(GL_ARRAY_BUFFER, m->vertexCnt * sizeof(StaticMeshVertex), m->vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("static mesh vbo load");
	
	return m;
}




MeshManager* meshManager_alloc() {
	MeshManager* mm;
	
	
	mm = calloc(1, sizeof(MeshManager));
	mm->meshes_alloc = 64;
	mm->meshes = malloc(mm->meshes_alloc * sizeof(StaticMesh*));
	mm->instances = malloc(mm->meshes_alloc * sizeof(StaticMeshInstance**));
	mm->inst_buf_info = calloc(1, mm->meshes_alloc * sizeof(struct buf_info));
	
//	glBindVertexArray(vao);
	
	
	//local vbo for collected mesh geometry
	//glGenBuffers(2, mm->instVBO);

}

// returns the index of the instance
int meshManager_addInstance(MeshManager* mm, int meshIndex, StaticMeshInstance* smi) {
	int a, c;
	
	if(meshIndex > mm->meshes_cnt) {
		return -1;
	}
	
	a = mm->inst_buf_info[meshIndex].alloc;
	c = mm->inst_buf_info[meshIndex].cnt;
	if(c >= a) {
		mm->instances[meshIndex] = realloc(mm->instances[meshIndex], a * 2 * sizeof(StaticMeshInstance));
		mm->inst_buf_info[meshIndex].alloc *= 2;
	}
	
	memcpy(&mm->instances[meshIndex][c], smi, sizeof(StaticMeshInstance));
	
	mm->inst_buf_info[meshIndex].cnt++;
	
	return c;
}

// returns the index if the mesh
int meshManager_addMesh(MeshManager* mm, StaticMesh* sm) {
	
	int i;
	
	if(mm->meshes_cnt >= mm->meshes_alloc) {
		mm->meshes = realloc(mm->meshes, mm->meshes_alloc * sizeof(StaticMesh*) * 2);
		mm->instances = realloc(mm->instances, mm->meshes_alloc * sizeof(StaticMeshInstance**) * 2);
		mm->inst_buf_info = realloc(mm->inst_buf_info, mm->meshes_alloc * sizeof(struct buf_info) * 2);
		mm->meshes_alloc *= 2;
	}
	
	//TODO: record offsets and lengths for rendering

	
	i = mm->meshes_cnt;
	
	mm->meshes[i] = sm;
	mm->meshes_cnt++;
	mm->totalVertices += sm->vertexCnt;
	
	mm->instances[i] = malloc(16 * sizeof(StaticMeshInstance));
	mm->inst_buf_info[i].alloc = 16;
	mm->inst_buf_info[i].cnt = 0;
	
	return i;
}

// should only used for initial setup
void meshManager_updateGeometry(MeshManager* mm) {
	
	int i, offset;
	
	glBindVertexArray(vao);
	
	if(glIsBuffer(mm->geomVBO)) glDeleteBuffers(1, &mm->geomVBO);
	glGenBuffers(1, &mm->geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	
	//GL_DYNAMIC_STORAGE_BIT so we can use glBufferSubData() later
	glBufferStorage(GL_ARRAY_BUFFER, mm->totalVertices * sizeof(StaticMeshVertex), NULL, GL_DYNAMIC_STORAGE_BIT);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*3*4 + 4, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*3*4 + 4, 1*3*4);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 2*3*4 + 4, 2*3*4);
	
	
	offset = 0;
	for(i = 0; i < mm->meshes_cnt; i++) {
		
		glBufferSubData(
			GL_ARRAY_BUFFER, 
			offset, 
			mm->meshes[i]->vertexCnt * sizeof(StaticMeshVertex), 
			mm->meshes[i]->vertices);
		//glBufferData(GL_ARRAY_BUFFER, m->vertexCnt * sizeof(StaticMeshVertex), m->vertices, GL_STATIC_DRAW);
	
		offset += mm->meshes[i]->vertexCnt * sizeof(StaticMeshVertex);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void meshManager_updateInstances(MeshManager* mm) {
	
	int i, mesh_index;
	StaticMeshInstance* buf_ptr;
	
	glBindVertexArray(vao);
	
	// TODO: deal with vbo cycling
	
	
	if(glIsBuffer(mm->geomVBO)) glDeleteBuffers(1, &mm->geomVBO);
	glGenBuffers(1, &mm->geomVBO);
	
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	
	//GL_DYNAMIC_STORAGE_BIT so we can use glBufferSubData() later
	glBufferStorage(GL_ARRAY_BUFFER, mm->totalVertices * sizeof(StaticMeshVertex), NULL, GL_DYNAMIC_STORAGE_BIT);
	
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3*3*4, 0);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3*3*4, 1*3*4);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3*3*4, 2*3*4);
	
	buf_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	// copy in data
	for(mesh_index = 0; mesh_index < mm->meshes_cnt; mesh_index++) {
	
		// TODO: offsets for rendering
		
		for(i = 0; i < mm->inst_buf_info[mesh_index].cnt; i++) {
			memcpy(buf_ptr, mm->instances[mesh_index][i], sizeof(StaticMeshInstance));
		}
	}
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void meshManager_draw(MeshManager* mm) {
	
	//multidrawindirect, client side 
}


