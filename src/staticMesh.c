 
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



static GLuint geom_vao, instance_vao;
static GLuint color_ul, model_ul, view_ul, proj_ul;
static ShaderProgram* prog;

void initStaticMeshes() {
	
	// VAO
	VAOConfig opts[] = {
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // normal
		{2, GL_UNSIGNED_SHORT}, // tex
		{0, 0}
	};
	
	geom_vao = makeVAO(opts, 2*3*4 + 2*2);

	VAOConfig opts[] = {
		{3, GL_FLOAT}, // position
		{3, GL_FLOAT}, // direction
		{3, GL_FLOAT}, // scale
		{0, 0}
	};
	
	instance_vao = makeVAO(opts, 3*3*4);
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
	
	glBindVertexArray(geom_vao);
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
	glBindVertexArray(geom_vao);
	
	glGenBuffers(1, &m->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 28, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 28, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 28, 24);

	glBufferData(GL_ARRAY_BUFFER, m->vertexCnt * sizeof(StaticMeshVertex), m->vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("static mesh vbo load");
	
	return m;
}




MeshManager* meshManager_alloc() {
	MeshManager* mm;
	
	
	mm = calloc(1, sizeof(MeshManager));
	mm->meshes_alloc = 64;
	mm->meshes = malloc(mm->meshes_alloc * sizeof(StaticMesh));
	
	// global vao for mesh geometry
	glBindVertexArray(geom_vao);
	
	//local vbo for collected mesh geometry
	glGenBuffers(1, &mm->geomVBO);
	glBindBuffer(GL_ARRAY_BUFFER, mm->geomVBO);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 28, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 28, 12);
	glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_TRUE, 28, 24);
	
	glBindVertexArray(0);
}





