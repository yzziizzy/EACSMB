#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "common_gl.h"

#include "ds.h"

#include "utilities.h"
#include "shader.h"


#include "debugWireframes.h"





typedef struct debugWireframeLine {
	Vector p1, p2;
	uint32_t color1, color2;
	float width1, width2;
} debugWireframeLine;

typedef struct debugWireframePoint {
	Vector p;
	uint32_t color;
	float size;
} debugWireframePoint;


static VEC(debugWireframeLine) debugLines;
static VEC(debugWireframePoint) debugPoints;



static GLuint debugVaoLines;
static GLuint debugVaoPoints;
static ShaderProgram* debugProgLines;
static ShaderProgram* debugProgPoints;

static GLuint wv_lines_ul, vp_lines_ul;
static GLuint wv_points_ul, vp_points_ul;


// VAO definitions
static VAOConfig vao_opts_lines[] = {
	// per vertex
	{0, 3, GL_FLOAT, 0, GL_FALSE}, // position
	{0, 4, GL_UNSIGNED_BYTE, 0, GL_TRUE}, // color
	
	{0, 0, 0}
};

static VAOConfig vao_opts_points[] = {
	// per vertex
	{0, 4, GL_FLOAT, 0, GL_FALSE}, // position, size
	{0, 4, GL_UNSIGNED_BYTE, 0, GL_TRUE}, // color
	{0, 0, 0}
};


struct debugLineShader {
	Vector p;
	uint32_t color;
};

struct debugPointShader {
	Vector p;
	uint32_t color;
};




void initDebugWireframe() {
	VEC_INIT(&debugLines);
	VEC_INIT(&debugPoints);
	
	debugVaoLines = makeVAO(vao_opts_lines);
	debugVaoPoints = makeVAO(vao_opts_points);
	
	debugProgLines = loadCombinedProgram("debugWireframeLines");
	debugProgPoints = loadCombinedProgram("debugWireframePoints");
	
	wv_lines_ul = glGetUniformLocation(debugProgLines->id, "mWorldView");
	vp_lines_ul = glGetUniformLocation(debugProgLines->id, "mViewProj");
	
	wv_points_ul= glGetUniformLocation(debugProgPoints->id, "mWorldView");
	vp_points_ul = glGetUniformLocation(debugProgPoints->id, "mViewProj");
	
	glProgramUniform1i(debugProgLines->id, glGetUniformLocation(debugProgLines->id, "sDepth"), 2);
	glProgramUniform1i(debugProgPoints->id, glGetUniformLocation(debugProgPoints->id, "sDepth"), 2);
}


void resetDebugWireframes() {
	VEC_TRUNC(&debugLines);
	VEC_TRUNC(&debugPoints);
}







void renderDebugWireframeLines(PassFrameParams* pfp) {
	GLuint vbo;
	
	if(0 == VEC_LEN(&debugLines)) return;
	
	glBindVertexArray(debugVaoLines);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	
	int stride = calcVAOStride(0, vao_opts_lines);
	updateVAO(0, vao_opts_lines); 
	
	glBufferStorage(GL_ARRAY_BUFFER, stride * VEC_LEN(&debugLines) * 2, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	glexit("");
	
	
	struct debugLineShader* buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	glexit("");
	
	
	VEC_EACH(&debugLines, i, line) {
		buf[i*2  ].p = line.p1;
		buf[i*2  ].color = line.color1;
		buf[i*2+1].p = line.p2;
		buf[i*2+1].color = line.color2;
	}
	
	
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glexit("");
	
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	
	glUseProgram(debugProgLines->id);
	glUniformMatrix4fv(wv_lines_ul, 1, GL_FALSE, pfp->dp->mWorldView->m);
	glUniformMatrix4fv(vp_lines_ul, 1, GL_FALSE, pfp->dp->mViewProj->m);

	glexit("");
// 	glBindVertexArray(vao);
	
// 	glDrawArrays(GL_LINES, 0, 12 * 2);
	glLineWidth(2);
	glDrawArrays(GL_LINES, 0, VEC_LEN(&debugLines) * 2);
	glexit("debug box draw");
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo);
	glexit("");
}



// TODO: parse color


void debugWF_Line(Vector* p1, Vector* p2, char* color1, char* color2, float width1, float width2) {
	uint32_t c1 = parseColor(color1);
	uint32_t c2 = parseColor(color2);
	
	VEC_PUSH(&debugLines, ((debugWireframeLine){
		.p1 = *p1,
		.p2 = *p2,
		.color1 = c1,
		.color2 = c2,
		.width1 = width1,
		.width2 = width2,
	}));
}

void debugWF_Ray(Vector* origin, Vector* dir, float len, char* color1, char* color2, float width1, float width2) {
	uint32_t c1 = parseColor(color1);
	uint32_t c2 = parseColor(color2);
	
	Vector p2;
	vScale(dir, len, &p2);
	vAdd(origin, &p2, &p2);
	
	VEC_PUSH(&debugLines, ((debugWireframeLine){
		.p1 = *origin,
		.p2 = p2,
		.color1 = c1,
		.color2 = c2,
		.width1 = width1,
		.width2 = width2,
	}));
}


void debugWF_ProjMatrix(Matrix* m) {
	
	Frustum f;
	
	static char* green = "green"; 
	static char* red = "red"; 
	
	frustumFromMatrix(m, &f);
	
	// near plane
	debugWF_Line(&f.points[0], &f.points[1], green, green, 1, 1);
	debugWF_Line(&f.points[0], &f.points[2], green, green, 1, 1);
	debugWF_Line(&f.points[3], &f.points[1], green, green, 1, 1);
	debugWF_Line(&f.points[3], &f.points[2], green, green, 1, 1);
	
	// far plane
	debugWF_Line(&f.points[4], &f.points[5], red, red, 1, 1);
	debugWF_Line(&f.points[4], &f.points[6], red, red, 1, 1);
	debugWF_Line(&f.points[7], &f.points[5], red, red, 1, 1);
	debugWF_Line(&f.points[7], &f.points[6], red, red, 1, 1);
	
	// connecting lines
	debugWF_Line(&f.points[0], &f.points[4], green, red, 1, 1);
	debugWF_Line(&f.points[1], &f.points[5], green, red, 1, 1);
	debugWF_Line(&f.points[2], &f.points[6], green, red, 1, 1);
	debugWF_Line(&f.points[3], &f.points[7], green, red, 1, 1);
}




void debugWF_AABB(AABB* aabb, char* color, float width) {
	Vector min[4] = { // the min plane on x
		{aabb->min.x, aabb->min.y, aabb->min.z},
		{aabb->min.x, aabb->min.y, aabb->max.z},
		{aabb->min.x, aabb->max.y, aabb->min.z},
		{aabb->min.x, aabb->max.y, aabb->max.z},
	};
	Vector max[4] = { // the max plane on x
		{aabb->max.x, aabb->min.y, aabb->min.z},
		{aabb->max.x, aabb->min.y, aabb->max.z},
		{aabb->max.x, aabb->max.y, aabb->min.z},
		{aabb->max.x, aabb->max.y, aabb->max.z},
	};
	
	debugWF_Line(&min[0], &min[1], color, color, width, width);
	debugWF_Line(&min[0], &min[2], color, color, width, width);
	debugWF_Line(&min[3], &min[1], color, color, width, width);
	debugWF_Line(&min[3], &min[2], color, color, width, width);

	debugWF_Line(&max[0], &max[1], color, color, width, width);
	debugWF_Line(&max[0], &max[2], color, color, width, width);
	debugWF_Line(&max[3], &max[1], color, color, width, width);
	debugWF_Line(&max[3], &max[2], color, color, width, width);
	
	debugWF_Line(&min[0], &max[0], color, color, width, width);
	debugWF_Line(&min[1], &max[1], color, color, width, width);
	debugWF_Line(&min[2], &max[2], color, color, width, width);
	debugWF_Line(&min[3], &max[3], color, color, width, width);
}



void debugWF_Sphere(Sphere* s, int segments, char* color, float width) {
	float th = (2 * F_PI) / (float)segments;
	Vector c = s->center;
	float r = s->r;
	
	char* red = color ? color : "red"; 
	char* blue = color ? color : "blue"; 
	char* green = color ? color : "green"; 
	
	for(int i = 0; i <= segments; i++) {
		float st = sin((float)i * th) * r;
		float ct = cos((float)i * th) * r;
		float st1 = sin((float)(i+1) * th) * r;
		float ct1 = cos((float)(i+1) * th) * r;
		
		// x plane
		debugWF_Line(&(Vector){c.x, c.y + st, c.z + ct}, &(Vector){c.x, c.y + st1, c.z + ct1}, red, red, width, width);
		// y plane
		debugWF_Line(&(Vector){c.x + st, c.y, c.z + ct}, &(Vector){c.x + st1, c.y, c.z + ct1}, blue, blue, width, width);
		// z plane
		debugWF_Line(&(Vector){c.x + st, c.y + ct, c.z}, &(Vector){c.x + st1, c.y + ct1, c.z}, green, green, width, width);
	}
	
	
}
