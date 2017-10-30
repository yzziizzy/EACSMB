


#include "waterPlane.h"

#include "utilities.h"


static GLuint vao;
static ShaderProgram* prog;
static GLuint color_ul, model_ul, view_ul, proj_ul;



void initWaterPlane() {
	
	// VAO
	VAOConfig opts[] = {
		// per vertex
		{3, GL_FLOAT}, // position
	//	{3, GL_FLOAT}, // normal is always up...
		{2, GL_UNSIGNED_SHORT}, // tex
		
		// per instance 
	//	{4, GL_FLOAT}, // position, scale
	//	{4, GL_FLOAT}, // direction, rotation
	//	{4, GL_FLOAT}, // alpha, x, x, x
		
		{0, 0}
	};
	
	vao = makeVAO(opts);
	
	glexit("static mesh vao");
	
	// shader
//  	prog = loadCombinedProgram("staticMesh");
	prog = loadCombinedProgram("waterPlane");
	
	model_ul = glGetUniformLocation(prog->id, "mModel");
	view_ul = glGetUniformLocation(prog->id, "mView");
	proj_ul = glGetUniformLocation(prog->id, "mProj");
	color_ul = glGetUniformLocation(prog->id, "color");
	
	glexit("");
	
//	tex = loadBitmapTexture("./assets/textures/gazebo-small.png");
	
//	glActiveTexture(GL_TEXTURE0 + 7);
//	glBindTexture(GL_TEXTURE_2D, tex->tex_id);
	
	glexit("");
}




void WaterPlane_draw(WaterPlane* wp, Matrix* view, Matrix* proj) {
	
	Matrix model;
	
	//mFastMul(view, proj, &mvp);
	mIdent(&model);
//	mScale3f(150, 150, 150, &model);
	//mTrans3f(0,0,0, &model);
	
	glUseProgram(prog->id);

	glUniformMatrix4fv(model_ul, 1, GL_FALSE, &model.m);
	glUniformMatrix4fv(view_ul, 1, GL_FALSE, &view->m);
	glUniformMatrix4fv(proj_ul, 1, GL_FALSE, &proj->m);
	glUniform3f(color_ul, .5, .2, .9);
	
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, wp->vbo);
	glexit("");
	
	glDrawArrays(GL_TRIANGLES, 0, wp->vertexCnt);
	glexit("");
}



void WaterPlane_create(WaterPlane* wp, float size, Vector* pos) {
	
	wp->vertexCnt = 6;
	
	glBindVertexArray(vao);
	
	glGenBuffers(1, &wp->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, wp->vbo);
	glexit("");
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*4 + 4, 0);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, 3*4 + 4, 3*4);

	glexit("");
	
	glBufferStorage(GL_ARRAY_BUFFER, wp->vertexCnt * sizeof(struct WaterPlaneVertex), NULL, GL_MAP_WRITE_BIT);
 
	struct WaterPlaneVertex* d = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	
	d[0].pos = (Vector){0,0,0};
	d[0].t = (UVPair){0,0};
	
	d[1].pos = (Vector){size,0,0};
	d[1].t = (UVPair){65535,0};
	
	d[2].pos = (Vector){size,0,size};
	d[2].t = (UVPair){65535, 65535};

	
	d[3].pos = (Vector){0,0,0};
	d[3].t = (UVPair){0,0};
	
	d[4].pos = (Vector){size,0,size};
	d[4].t = (UVPair){65535, 65535};
	
	d[5].pos = (Vector){0,0,size};
	d[5].t = (UVPair){0,65535};
	
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
 
	
	glexit("");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glexit("");
}


